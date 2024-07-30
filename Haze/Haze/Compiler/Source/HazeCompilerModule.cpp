#include "HazePch.h"

#include "Parse.h"
#include "HazeTokenText.h"
#include "HazeDebugDefine.h"
#include "HazeLogDefine.h"
#include "HazeFilePathHelper.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerPointerFunction.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerInitlistValue.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerEnum.h"
#include "HazeCompilerEnumValue.h"

struct PushTempRegister
{
	PushTempRegister(HAZE_STRING_STREAM& hss, HazeCompiler* compiler, HazeCompilerModule* compilerModule)
		: Size(0), Hss(hss), Compiler(compiler), Module(compilerModule)
	{
		UseTempRegisters = Compiler->GetUseTempRegister();
		for (auto& regi : UseTempRegisters)
		{
			Hss << GetInstructionString(InstructionOpCode::PUSH) << " ";
			GenVariableHzic(Module, Hss, regi.second);
			Hss << std::endl;
			Size += regi.second->GetSize();
		}
	}

	~PushTempRegister()
	{
		for (auto& regi : UseTempRegisters)
		{
			Hss << GetInstructionString(InstructionOpCode::POP) << " ";
			GenVariableHzic(Module, Hss, regi.second);
			Hss << std::endl;
		}
		
		if (Compiler->GetInsertBlock())
		{
			Compiler->GetInsertBlock()->PushIRCode(Hss.str());
		}
		else
		{
			Module->m_ModuleIRCodes.push_back(Hss.str());
		}

		Compiler->ResetTempRegister(UseTempRegisters);
	}

	int GetSize() const { return Size; }
private:
	int Size;
	HAZE_STRING_STREAM& Hss;
	HazeCompiler* Compiler;
	HazeCompilerModule* Module;
	HashMap<const HChar*, Share<HazeCompilerValue>> UseTempRegisters;
};


HazeCompilerModule::HazeCompilerModule(HazeCompiler* compiler, const HString& moduleName, const HString& modulePath)
	: m_Compiler(compiler), m_ModuleLibraryType(HazeLibraryType::Normal), m_IsBeginCreateFunctionVariable(false),
	 m_IsGenTemplateCode(false), m_Path(modulePath)
{
#if HAZE_I_CODE_ENABLE

	m_FS_I_Code.imbue(std::locale("chs"));
	m_FS_I_Code.open(GetIntermediateModuleFile(moduleName));

#endif
}

HazeCompilerModule::~HazeCompilerModule()
{
	if (m_FS_I_Code.is_open())
	{
		m_FS_I_Code.close();
	}
}

const HString& HazeCompilerModule::GetName() const
{
	return *m_Compiler->GetModuleName(this);
}

void HazeCompilerModule::MarkLibraryType(HazeLibraryType type)
{
	m_ModuleLibraryType = type;
}

void HazeCompilerModule::RestartTemplateModule(const HString& moduleName)
{
#if HAZE_I_CODE_ENABLE
	
	if (!m_IsGenTemplateCode)
	{
		m_FS_I_Code.imbue(std::locale("chs"));
		m_FS_I_Code.open(GetIntermediateModuleFile(moduleName));
		m_IsGenTemplateCode = true;
	}

#endif
}

void HazeCompilerModule::FinishModule()
{
	m_CurrFunction.clear();
}

void HazeCompilerModule::GenCodeFile()
{
#if HAZE_I_CODE_ENABLE

	//生成中间代码先不需要计算symbol table表中的偏移，等统一生成字节码时在进行替换，模板会重新打开文件流。
	if (m_FS_I_Code.is_open())
	{
		GenICode();
		m_FS_I_Code.close();
	}

#endif
}

Share<HazeCompilerClass> HazeCompilerModule::CreateClass(const HString& name, V_Array<HazeCompilerClass*>& parentClass,
	V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<HazeCompilerValue>>>>>& classData)
{
	auto compilerClass = GetClass(name);
	if (!compilerClass)
	{
		m_HashMap_Classes[name] = MakeShare<HazeCompilerClass>(this, name, parentClass, classData);
		compilerClass = m_HashMap_Classes[name];

		m_Compiler->OnCreateClass(compilerClass);
		compilerClass->InitThisValue();
	}

	m_CurrClass = name;
	return compilerClass;
}

Share<HazeCompilerEnum> HazeCompilerModule::CreateEnum(const HString& name, HazeValueType baseType)
{
	Share<HazeCompilerEnum> compilerEnum = GetEnum(this, name);
	if (!compilerEnum)
	{
		compilerEnum = MakeShare<HazeCompilerEnum>(this, name, baseType);
		m_HashMap_Enums[name] = compilerEnum;
	
		m_CurrEnum = name;
	}

	return compilerEnum;
}

Share<HazeCompilerEnum> HazeCompilerModule::GetCurrEnum()
{
	auto iter = m_HashMap_Enums.find(m_CurrEnum);
	if (iter != m_HashMap_Enums.end())
	{
		return iter->second;
	}

	return nullptr;
}

void HazeCompilerModule::FinishCreateEnum()
{
	m_CurrEnum.clear();
}

void HazeCompilerModule::FinishCreateClass()
{
	m_CurrClass.clear();
}

Share<HazeCompilerFunction> HazeCompilerModule::GetCurrFunction()
{
	if (m_CurrFunction.empty())
	{
		return nullptr;
	}

	auto iter = m_HashMap_Functions.find(m_CurrFunction);
	if (iter == m_HashMap_Functions.end())
	{
		if (m_CurrClass.empty())
		{
			return nullptr;
		}
		else
		{
			return GetClass(m_CurrClass)->FindFunction(m_CurrFunction);
		}
	}

	return iter->second;
}

Pair<Share<HazeCompilerFunction>, Share<HazeCompilerValue>> HazeCompilerModule::GetFunction(const HString& name)
{
	auto it = m_HashMap_Functions.find(name);
	if (it != m_HashMap_Functions.end())
	{
		return { it->second, nullptr };
	}
	else if (!m_CurrClass.empty())
	{
		auto iter = m_HashMap_Classes.find(m_CurrClass);
		if (iter != m_HashMap_Classes.end())
		{
			auto func = iter->second->FindFunction(name);
			if (func)
			{
				return { func, nullptr };
			}

		}
	}

	bool isPointer;
	return GetObjectFunction(this, name, isPointer);
}

void HazeCompilerModule::StartCacheTemplate(HString& templateName, uint32 startLine, HString& templateText, V_Array<HString>& templateTypes)
{
	auto iter = m_HashMap_TemplateText.find(templateName);
	if (iter == m_HashMap_TemplateText.end())
	{
		m_HashMap_TemplateText.insert({ Move(templateName), { startLine, Move(templateText),
			Move(templateTypes) } });
	}
}

bool HazeCompilerModule::IsTemplateClass(const HString& name)
{
	auto iter = m_HashMap_TemplateText.find(name);
	if (iter != m_HashMap_TemplateText.end())
	{
		return true;
	}

	for (auto& it : m_ImportModules)
	{
		if (it->IsTemplateClass(name))
		{
			return true;
		}
	}

	return false;
}

bool HazeCompilerModule::ResetTemplateClassRealName(HString& inName, const V_Array<HazeDefineType>& templateTypes)
{
	for (auto& templateText : m_HashMap_TemplateText)
	{
		if (templateText.first == inName)
		{
			if (templateTypes.size() != templateText.second.Types.size())
			{
				HAZE_LOG_ERR_W("生成模板<%s>错误, 类型数应为%d, 实际为%d!\n", inName.c_str(), templateText.second.Types.size(), 
					templateTypes.size());
				return false;
			}

			//HString className = inName;
			//GetTemplateClassName(inName, templateTypes);

			for (auto& compilerClass : m_HashMap_Classes)
			{
				if (compilerClass.first == inName)
				{
					return true;
				}
			}

			Parse p(m_Compiler);
			p.InitializeString(templateText.second.Text, templateText.second.StartLine);
			p.ParseTemplateContent(GetName(), inName, templateText.second.Types, templateTypes);
			return true;
		}
	}

	for (auto& m : m_ImportModules)
	{
		if (m->ResetTemplateClassRealName(inName, templateTypes))
		{
			return true;
		}
	}

	return false;
}

Share<HazeCompilerEnum> HazeCompilerModule::GetEnum(HazeCompilerModule* m, const HString& name)
{
	auto ret = m->GetEnum_Internal(name);
	if (ret)
	{
		return ret;
	}

	return m->m_Compiler->GetBaseModuleEnum(name);
}

Share<HazeCompilerFunction> HazeCompilerModule::CreateFunction(const HString& name, HazeDefineType& type, V_Array<HazeDefineVariable>& params)
{
	Share<HazeCompilerFunction> function = nullptr;
	auto it = m_HashMap_Functions.find(name);
	if (it == m_HashMap_Functions.end())
	{
		m_HashMap_Functions[name] = MakeShare<HazeCompilerFunction>(this, name, type, params);
		m_HashMap_Functions[name]->InitEntryBlock(HazeBaseBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, m_HashMap_Functions[name], nullptr));

		function = m_HashMap_Functions[name];
	}
	else
	{
		function = it->second;
	}

	m_CurrFunction = name;
	return function;
}

Share<HazeCompilerFunction> HazeCompilerModule::CreateFunction(Share<HazeCompilerClass> compilerClass, const HString& name,
	HazeDefineType& type, V_Array<HazeDefineVariable>& params)
{
	Share<HazeCompilerFunction> function = compilerClass->FindFunction(name);
	if (!function)
	{
		function = MakeShare<HazeCompilerFunction>(this, GetHazeClassFunctionName(compilerClass->GetName(), name), type, params, compilerClass.get());
		compilerClass->AddFunction(function);

		function->InitEntryBlock(HazeBaseBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, function, nullptr));
	}

	m_CurrFunction = name;
	return function;
}

void HazeCompilerModule::FinishFunction()
{
	GetCurrFunction()->FunctionFinish();
	m_CurrFunction.clear();
}

Share<HazeCompilerValue> HazeCompilerModule::CreateAdd(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::ADD_ASSIGN : InstructionOpCode::ADD);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateSub(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::SUB_ASSIGN : InstructionOpCode::SUB);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateMul(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::MUL_ASSIGN : InstructionOpCode::MUL);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateDiv(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::DIV_ASSIGN : InstructionOpCode::DIV);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateMod(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::MOD_ASSIGN : InstructionOpCode::MOD);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateBitAnd(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::BIT_AND_ASSIGN : InstructionOpCode::BIT_AND);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateBitOr(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::BIT_OR_ASSIGN : InstructionOpCode::BIT_OR);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateBitNeg(Share<HazeCompilerValue> value)
{
	return GenIRCode_BinaryOperater(value, nullptr, InstructionOpCode::BIT_NEG);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateBitXor(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::BIT_XOR_ASSIGN : InstructionOpCode::BIT_XOR);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateShl(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::SHL_ASSIGN : InstructionOpCode::SHL);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateShr(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::SHR_ASSIGN : InstructionOpCode::SHR);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateNot(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right)
{
	return GenIRCode_BinaryOperater(left, right, InstructionOpCode::NOT);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateNeg(Share<HazeCompilerValue> value)
{
	if (value->IsConstant())
	{
		value = m_Compiler->GenConstantValue(value->GetValueType().PrimaryType, GetNegValue(value->GetValueType().PrimaryType, value->GetValue()));
	}
	else
	{
		auto tempRegister = m_Compiler->GetTempRegister();
		m_Compiler->CreateMov(tempRegister, value);

		HAZE_STRING_STREAM ss;
		ss << GetInstructionString(InstructionOpCode::NEG) << " ";
		GenVariableHzic(this, ss, tempRegister);
		ss << std::endl;

		Share<HazeBaseBlock> BB = m_Compiler->GetInsertBlock();
		BB->PushIRCode(ss.str());

		value = tempRegister;
	}

	return value;
}

Share<HazeCompilerValue> HazeCompilerModule::CreateInc(Share<HazeCompilerValue> value, bool isPreInc)
{
	Share<HazeCompilerValue> retValue = value;
	if (IsHazeBaseType(value->GetValueType().PrimaryType))
	{
		if (!isPreInc)
		{
			retValue = m_Compiler->GetTempRegister();
			m_Compiler->CreateMov(retValue, value);
		}
		//Compiler->CreateMov(Value, CreateAdd(Value, Compiler->GetConstantValueInt(1)));
		GenIRCode_UnaryOperator(value, InstructionOpCode::INC);
	}
	else
	{
		HAZE_LOG_ERR_W("<%s>类型不能使用Inc操作\n", GetHazeValueTypeString(value->GetValueType().PrimaryType));
	}
	return retValue;
}

Share<HazeCompilerValue> HazeCompilerModule::CreateDec(Share<HazeCompilerValue> value, bool isPreDec)
{
	Share<HazeCompilerValue> retValue = value;
	if (IsHazeBaseType(value->GetValueType().PrimaryType))
	{
		if (!isPreDec)
		{
			retValue = m_Compiler->GetTempRegister();
			m_Compiler->CreateMov(retValue, value);
		}
		//Compiler->CreateMov(Value, CreateSub(Value, Compiler->GetConstantValueInt(1)));
		GenIRCode_UnaryOperator(value, InstructionOpCode::DEC);
	}
	else
	{
		HAZE_LOG_ERR_W("<%s>类型不能使用Dec操作\n", GetHazeValueTypeString(value->GetValueType().PrimaryType));
	}
	return retValue;
}

Share<HazeCompilerValue> HazeCompilerModule::CreateArrayInit(Share<HazeCompilerValue> array, Share<HazeCompilerValue> initList)
{
	auto arrayValue = DynamicCast<HazeCompilerArrayValue>(array);
	auto initListValue = DynamicCast<HazeCompilerInitListValue>(initList);

	HAZE_STRING_STREAM hss;
	HString varName;

	if (IsHazeBaseType(arrayValue->GetValueType().SecondaryType))
	{
		for (int i = 0; i < (int)arrayValue->GetArrayLength(); i++)
		{
			hss << GetInstructionString(InstructionOpCode::MOV) << " ";

			//GenVariableHzic(this, hss, array, i);

			hss << " ";

			GenVariableHzic(this, hss, initListValue->GetList()[i]);

			hss << std::endl;

			Share<HazeBaseBlock> BB = m_Compiler->GetInsertBlock();
			BB->PushIRCode(hss.str());
			hss.str(H_TEXT(""));
		}
	}
	else
	{
		HAZE_LOG_ERR_W("目前只支持Haze默认类型的数组初始化赋值!\n");
	}

	return array;
}

Share<HazeCompilerValue> HazeCompilerModule::GenIRCode_BinaryOperater(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, InstructionOpCode opCode)
{
	static HashSet<InstructionOpCode> s_HashSet_NoTemp =
	{
		InstructionOpCode::MOV,
		InstructionOpCode::MOVPV,
		InstructionOpCode::MOVTOPV,
		InstructionOpCode::LEA,
		InstructionOpCode::CMP,
		InstructionOpCode::ADD_ASSIGN,
		InstructionOpCode::SUB_ASSIGN,
		InstructionOpCode::MUL_ASSIGN,
		InstructionOpCode::DIV_ASSIGN,
		InstructionOpCode::MOD_ASSIGN,
		InstructionOpCode::BIT_AND_ASSIGN,
		InstructionOpCode::BIT_OR_ASSIGN,
		InstructionOpCode::BIT_XOR_ASSIGN,
		InstructionOpCode::SHL_ASSIGN,
		InstructionOpCode::SHR_ASSIGN,
		InstructionOpCode::CVT,
	};

	Share<HazeCompilerValue> retValue = left;

	if (left->IsConstant() && right->IsConstant())
	{
		if (IsNumberType(left->GetValueType().PrimaryType))
		{
			auto& leftValue = const_cast<HazeValue&>(left->GetValue());
			HazeValue tempValue = leftValue;
			auto& rightValue = const_cast<HazeValue&>(right->GetValue());
			CalculateValueByType(left->GetValueType().PrimaryType, opCode, &rightValue, &leftValue);

			retValue = m_Compiler->GenConstantValue(left->GetValueType().PrimaryType, leftValue);
			leftValue = tempValue;
		}
		else
		{
			COMPILER_ERR_MODULE_W("生成<%s>操作错误", GetInstructionString(opCode), GetName().c_str());
		}
		return retValue;
	}

	bool needTemp = s_HashSet_NoTemp.find(opCode) == s_HashSet_NoTemp.end();

	HAZE_STRING_STREAM ss;
	
	if (!m_CurrFunction.empty())
	{
		if (needTemp)
		{
			if ((!left->IsRegister(HazeDataDesc::RegisterTemp) && right == nullptr) ||
				(!left->IsRegister(HazeDataDesc::RegisterTemp) && !right->IsRegister(HazeDataDesc::RegisterTemp)))
			{
				retValue = m_Compiler->CreateMov(m_Compiler->GetTempRegister(), left);
			}
			else
			{
				if (right->IsRegister(HazeDataDesc::RegisterTemp) && right->GetValueType() == left->GetValueType()
					&& !left->IsRegister(HazeDataDesc::RegisterTemp))
				{
					retValue = right;
					right = left;
				}
			}
		}

		GenIRCode(ss, this, opCode, retValue, right);
		m_Compiler->GetInsertBlock()->PushIRCode(ss.str());
	}
	else if (left->IsGlobalVariable())
	{
		GenIRCode(ss, this, opCode, retValue, right);
		m_ModuleIRCodes.push_back(ss.str());
	}

	return retValue;
}

void HazeCompilerModule::GenIRCode_UnaryOperator(Share<HazeCompilerValue> value, InstructionOpCode opCode)
{
	auto function = GetCurrFunction();
	HAZE_STRING_STREAM hss;

	if (m_CurrFunction.empty())
	{
		HAZE_TO_DO(全局语句暂时不处理);
	}
	else
	{
		hss << GetInstructionString(opCode) << " ";
		GenVariableHzic(this, hss, value);

		hss << std::endl;
	}

	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void HazeCompilerModule::GenIRCode_Ret(Share<HazeCompilerValue> value)
{
	auto function = GetCurrFunction();
	HAZE_STRING_STREAM hss;
	hss << GetInstructionString(InstructionOpCode::RET) << " ";
	GenVariableHzic(this, hss, value);

	hss << std::endl;

	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void HazeCompilerModule::GenIRCode_Cmp(HazeCmpType cmpType, Share<HazeBaseBlock> ifJmpBlock, Share<HazeBaseBlock> elseJmpBlock)
{
	HAZE_STRING_STREAM hss;

	if (cmpType == HazeCmpType::None)
	{
		HAZE_LOG_ERR_W("比较失败,比较类型为空,当前函数<%s>!\n", GetCurrFunction()->GetName().c_str());
	}

	hss << GetInstructionString(GetInstructionOpCodeByCmpType(cmpType)) << " ";

	if (ifJmpBlock)
	{
		hss << ifJmpBlock->GetName() << " ";
	}
	else
	{
		hss << HAZE_JMP_NULL << " ";
	}

	if (elseJmpBlock)
	{
		hss << elseJmpBlock->GetName();
	}
	else
	{
		hss << HAZE_JMP_NULL << " ";
	}

	hss << std::endl;

	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void HazeCompilerModule::GenIRCode_JmpTo(Share<HazeBaseBlock> block)
{
	HAZE_STRING_STREAM hss;
	hss << GetInstructionString(InstructionOpCode::JMP) << " " << block->GetName() << std::endl;

	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

Share<HazeCompilerValue> HazeCompilerModule::CreateGlobalVariable(const HazeDefineVariable& var, Share<HazeCompilerValue> refValue,
	V_Array<Share<HazeCompilerValue>> arraySize, V_Array<HazeDefineType>* params)
{
	for (auto& it : m_Variables)
	{
		if (it.first == var.Name)
		{
			HAZE_LOG_ERR_W("编译器错误,添加全局变量重复\n");
			return nullptr;
		}
	}

	m_Variables.push_back({ var.Name,
		CreateVariable(this, var, HazeVariableScope::Global, HazeDataDesc::None, 0, refValue, arraySize, params) });

	auto& retValue = m_Variables.back().second;
	if (!IsHazeBaseType(retValue->GetValueType().PrimaryType))
	{
		if (retValue->IsClass())
		{

		}
	}



	return retValue;
}

void HazeCompilerModule::FunctionCall(HAZE_STRING_STREAM& hss, Share<HazeCompilerFunction> callFunction, Share<HazeCompilerValue> pointerFunction,
	AdvanceFunctionInfo* advancFunctionInfo, uint32& size, V_Array<Share<HazeCompilerValue>>& params,
	Share<HazeCompilerValue> thisPointerTo)
{
	Share<HazeBaseBlock> insertBlock = m_Compiler->GetInsertBlock();
	HString strName;

	auto pointerFunc = DynamicCast<HazeCompilerPointerFunction>(pointerFunction);

	V_Array<const HazeDefineType*> funcTypes(params.size());
	for (int64 i = params.size() - 1; i >= 0; i--)
	{
		auto Variable = params[i];

		if (!callFunction && !pointerFunc && !advancFunctionInfo)
		{
			COMPILER_ERR_MODULE_W("生成函数调用错误, <%s>为空", GetName().c_str(),
				callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"));
		}
		else
		{
			auto& type = callFunction ? callFunction->GetParamTypeLeftToRightByIndex(params.size() - 1 - i) : 
				pointerFunc ? pointerFunc->GetParamTypeLeftToRightByIndex(params.size() - 1 - i) :
				advancFunctionInfo->Params.at(params.size() - 1 - i);

			if (type != Variable->GetValueType() && !Variable->GetValueType().IsStrongerType(type))
			{
				if (Variable->IsEnum())
				{
					auto enumValue = DynamicCast<HazeCompilerEnumValue>(Variable);
					if (enumValue && enumValue->GetEnum() && enumValue->GetEnum()->GetParentType() == type.PrimaryType) {}
					else
					{
						COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数枚举类型不匹配", GetName().c_str(),
							callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"), params.size() - 1 - i);
					}
				}
				else if (type.IsStrongerType(Variable->GetValueType())) { }
				else if (IsRefrenceType(type.PrimaryType) && Variable->GetValueType().PrimaryType == type.SecondaryType) {}
				else if (!IsMultiVariableTye(type.PrimaryType))
				{
					COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数类型不匹配", GetName().c_str(),
						callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"),
						params.size() - 1 - i);
				}
			}

			funcTypes[i] = &type;
		}
	}

	for (size_t i = 0; i < params.size(); i++)
	{
		GenIRCode(hss, this, InstructionOpCode::PUSH, params[i], nullptr, funcTypes[i]);

		if (insertBlock)
		{
			insertBlock->PushIRCode(hss.str());
		}
		else
		{
			m_ModuleIRCodes.push_back(hss.str());
		}

		hss.str(H_TEXT(""));

		size += params[i]->GetSize();
		strName.clear();
	}

	if (thisPointerTo)
	{
		GenIRCode(hss, this, InstructionOpCode::PUSH, thisPointerTo);

		if (insertBlock)
		{
			insertBlock->PushIRCode(hss.str());
		}
		else
		{
			m_ModuleIRCodes.push_back(hss.str());
		}

		hss.str(H_TEXT(""));

		size += GetSizeByHazeType(HazeValueType::Class);
	}

	hss << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CALL_PUSH_ADDRESS_NAME << " " << CAST_SCOPE(HazeVariableScope::None)
		<< " " << (uint32)HazeDataDesc::Address << " " << CAST_TYPE(HazeValueType::Int32) << std::endl;
	if (insertBlock)
	{
		insertBlock->PushIRCode(hss.str());
	}
	else
	{
		m_ModuleIRCodes.push_back(hss.str());
	}
	hss.str(H_TEXT(""));
}

Share<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(Share<HazeCompilerFunction> callFunction, 
	V_Array<Share<HazeCompilerValue>>& params, Share<HazeCompilerValue> thisPointerTo)
{
	HAZE_STRING_STREAM hss;
	uint32 size = 0;

	PushTempRegister pushTempRegister(hss, m_Compiler, this);
	FunctionCall(hss, callFunction, nullptr, nullptr, size, params, thisPointerTo);

	hss << GetInstructionString(InstructionOpCode::CALL) << " " << callFunction->GetName() << " " << CAST_TYPE(HazeValueType::None) 
		<< " " << CAST_SCOPE(HazeVariableScope::Ignore) << " " << CAST_DESC(HazeDataDesc::FunctionAddress)
		<< " " << params.size() << " " << size << " " << callFunction->GetModule()->GetName() << " "
		<< CAST_DESC(HazeDataDesc::CallFunctionModule) << std::endl;

	auto retRegister = HazeCompiler::GetRegister(RET_REGISTER);

	auto& retRegisterType = const_cast<HazeDefineType&>(retRegister->GetValueType());
	retRegisterType = callFunction->GetFunctionType();
	return retRegister;
}

Share<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(Share<HazeCompilerValue> pointerFunction, 
	V_Array<Share<HazeCompilerValue>>& params, Share<HazeCompilerValue> thisPointerTo)
{
	Share<HazeBaseBlock> insertBlock = m_Compiler->GetInsertBlock();
	HAZE_STRING_STREAM hss;
	uint32 size = 0;

	HString varName;
	GetGlobalVariableName(this, pointerFunction, varName);
	if (varName.empty())
	{
		GetCurrFunction()->FindLocalVariableName(pointerFunction.get(), varName);
		if (varName.empty())
		{
			HAZE_LOG_ERR_W("函数指针调用失败!\n");
			return nullptr;
		}
	}

	PushTempRegister pushTempRegister(hss, m_Compiler, this);
	FunctionCall(hss, nullptr, pointerFunction, nullptr, size, params, thisPointerTo);
	hss << GetInstructionString(InstructionOpCode::CALL) << " " << varName << " " << CAST_TYPE(HazeValueType::Function) << " "
		<< CAST_SCOPE(pointerFunction->GetVariableScope())  << " " << CAST_DESC(pointerFunction->GetVariableDesc()) << " " << params.size()
		<< " " << size << " " << m_Compiler->GetCurrModuleName() << " " << CAST_DESC(HazeDataDesc::CallFunctionModule) << std::endl;

	return HazeCompiler::GetRegister(RET_REGISTER);
}

Share<HazeCompilerValue> HazeCompilerModule::CreateAdvanceTypeFunctionCall(AdvanceFunctionInfo& functionInfo, 
	V_Array<Share<HazeCompilerValue>>& params, Share<HazeCompilerValue> thisPointerTo)
{
	Share<HazeBaseBlock> insertBlock = m_Compiler->GetInsertBlock();
	HAZE_STRING_STREAM hss;
	uint32 size = 0;

	HString varName;
	GetGlobalVariableName(this, thisPointerTo, varName);
	if (varName.empty())
	{
		GetCurrFunction()->FindLocalVariableName(thisPointerTo.get(), varName);
		if (varName.empty())
		{
			HAZE_LOG_ERR_W("函数指针调用失败!\n");
			return nullptr;
		}
	}

	PushTempRegister pushTempRegister(hss, m_Compiler, this);
	FunctionCall(hss, nullptr, nullptr, &functionInfo, size, params, thisPointerTo);
	hss << GetInstructionString(InstructionOpCode::CALL) << " " << varName << " " << CAST_TYPE(HazeValueType::Function) << " "
		<< CAST_SCOPE(thisPointerTo->GetVariableScope()) << " " << CAST_DESC(thisPointerTo->GetVariableDesc()) << " " << params.size()
		<< " " << size << " " << m_Compiler->GetCurrModuleName() << " " << CAST_DESC(HazeDataDesc::CallFunctionPointer)
		<< " " << functionInfo.Func << std::endl;

	return HazeCompiler::GetRegister(RET_REGISTER);
}

Share<HazeCompilerValue> HazeCompilerModule::GetOrCreateGlobalStringVariable(const HString& str)
{
	auto it = m_HashMap_StringTable.find(str);
	if (it != m_HashMap_StringTable.end())
	{
		return it->second;
	}
	m_HashMap_StringTable[str] = nullptr;

	it = m_HashMap_StringTable.find(str);

	m_HashMap_StringMapping[(int)m_HashMap_StringMapping.size()] = &it->first;

	HazeDefineVariable defineVar;
	defineVar.Type.PrimaryType = HazeValueType::String;

	it->second = CreateVariable(this, defineVar, HazeVariableScope::Global, HazeDataDesc::ConstantString, 0);

	HazeValue& hazeValue = const_cast<HazeValue&>(it->second->GetValue());
	hazeValue.Value.Extra.StringTableIndex = (int)m_HashMap_StringMapping.size() - 1;

	return it->second;
}

uint32 HazeCompilerModule::GetGlobalStringIndex(Share<HazeCompilerValue> value)
{
	for (auto& it : m_HashMap_StringTable)
	{
		if (value == it.second)
		{
			for (auto& It : m_HashMap_StringMapping)
			{
				if (&it.first == It.second)
				{
					return It.first;
				}
			}
		}
	}

	return 0;
}

void HazeCompilerModule::PreStartCreateGlobalVariable()
{
	m_VariablesAddress.push_back({ (int)m_ModuleIRCodes.size(), 0 });
}

void HazeCompilerModule::EndCreateGlobalVariable()
{
	m_VariablesAddress[m_VariablesAddress.size() - 1].second = (int)m_ModuleIRCodes.size();
}

Share<HazeCompilerValue> HazeCompilerModule::GetGlobalVariable(HazeCompilerModule* m, const HString& name)
{
	auto ret = m->GetGlobalVariable_Internal(name);
	if (ret)
	{
		return ret;
	}

	return m->m_Compiler->GetBaseModuleGlobalVariable(name);
}

bool HazeCompilerModule::GetGlobalVariableName(HazeCompilerModule* m, const Share<HazeCompilerValue>& value, HString& outName, bool getOffset, V_Array<uint64>* offsets)
{
	if (m->GetGlobalVariableName_Internal(value, outName, getOffset, offsets))
	{
		return true;
	}

	if (m->m_Compiler->GetBaseModuleGlobalVariableName(value, outName))
	{
		return true;
	}

	return value->TryGetVariableName(outName);
}

Share<HazeCompilerValue> HazeCompilerModule::GetGlobalVariable_Internal(const HString& name)
{
	for (auto& it : m_Variables)
	{
		if (it.first == name)
		{
			return it.second;
		}
		else if (it.second->IsClass())
		{
			auto ret = GetObjectMember(this, name);
			if (ret)
			{
				return ret;
			}
		}
	}

	for (auto& m : m_ImportModules)
	{
		auto ret = m->GetGlobalVariable_Internal(name);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

bool HazeCompilerModule::GetGlobalVariableName_Internal(const Share<HazeCompilerValue>& value, HString& outName, bool getOffset, V_Array<uint64>* offsets)
{
	if (value->IsRegister())
	{
		outName = m_Compiler->GetRegisterName(value);
		return true;
	}

	for (auto& it : m_Variables)
	{
		if (TrtGetVariableName(nullptr, it, value.get(), outName))
		{
			return true;
		}
	}

	for (auto& it : m_ImportModules)
	{
		if (it->GetGlobalVariableName_Internal(value, outName, getOffset, offsets))
		{
			return true;
		}
	}

	for (auto& it : m_HashMap_StringTable)
	{
		if (it.second == value)
		{
			outName = HAZE_CONSTANT_STRING_NAME;//It.first;
			return true;
		}
	}

	return value->TryGetVariableName(outName);
}

Share<HazeCompilerEnum> HazeCompilerModule::GetEnum_Internal(const HString& name)
{
	auto iter = m_HashMap_Enums.find(name);
	if (iter != m_HashMap_Enums.end())
	{
		return iter->second;
	}

	for (auto& m : m_ImportModules)
	{
		auto ret = m->GetEnum_Internal(name);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

Share<HazeCompilerClass> HazeCompilerModule::GetClass(const HString& className)
{
	auto iter = m_HashMap_Classes.find(className);
	if (iter != m_HashMap_Classes.end())
	{
		return iter->second;
	}

	for (auto& it : m_ImportModules)
	{
		auto ret = it->GetClass(className);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

uint32 HazeCompilerModule::GetClassSize(const HString& className)
{
	return GetClass(className)->GetDataSize();
}

void HazeCompilerModule::GenICode()
{
	//版本 2个字节
	//FS_Ass << "1 1" << std::endl;

	//堆栈 4个字节
	//FS_Ass << 1024 << std::endl;

	//库类型
	HAZE_STRING_STREAM hss;
	hss << (uint32)m_ModuleLibraryType << std::endl;
	/*
	*	全局数据 ：	个数
	*				数据名 数据类型 数据
	*/
	hss << GetGlobalDataHeaderString() << std::endl;
	hss << m_Variables.size() << std::endl;

	for (int i = 0; i < m_Variables.size(); i++)
	{
		auto& var = m_Variables[i];
	
		hss << m_VariablesAddress[i].first << " " << m_VariablesAddress[i].second << " ";

		hss << var.first << " " << var.second->GetSize() << " ";
		var.second->GetValueType().StringStreamTo(hss);

		hss << std::endl;
	}

	//全局变量初始化
	hss << GetGlobalDataInitBlockStart() << std::endl;
	for (size_t i = 0; i < m_ModuleIRCodes.size(); i++)
	{
		hss << m_ModuleIRCodes[i].c_str();
	}
	hss << GetGlobalDataInitBlockEnd() << std::endl;
	

	/*
	*	字符串表 ：	个数
	*				字符串长度 字符串
	*/
	if (m_HashMap_StringMapping.size() != m_HashMap_StringTable.size())
	{
		HAZE_LOG_ERR_W("生成字符串表失败!",
			m_HashMap_StringMapping.size(), m_HashMap_StringTable.size());
		return;
	}
	hss << GetStringTableHeaderString() << std::endl;
	hss << m_HashMap_StringMapping.size() << std::endl;

	for (auto& it : m_HashMap_StringMapping)
	{
		hss << it.second->length() << " " << *it.second << std::endl;
	}

	/*
	*	类表 ：	个数
	*				名称 指令流
	*
	*/
	size_t functionSize = 0;

	hss << GetClassTableHeaderString() << std::endl;
	hss << m_HashMap_Classes.size() << std::endl;

	for (auto& iter : m_HashMap_Classes)
	{
		iter.second->GenClassData_I_Code(hss);
		functionSize += iter.second->GetFunctionNum();
	}

	/*
	*	函数表 ：	个数
	*				名称 指令流
	*
	*/
	hss << GetFucntionTableHeaderString() << std::endl;
	hss << m_HashMap_Functions.size() + functionSize << std::endl;

	for (auto& iter : m_HashMap_Classes)
	{
		iter.second->GenClassFunction_I_Code(hss);
	}

	for (auto& iter : m_HashMap_Functions)
	{
		iter.second->GenI_Code(hss);
	}

	m_FS_I_Code << hss.str();

	if (m_IsGenTemplateCode)
	{
		m_IsGenTemplateCode = false;
	}
}