#include "HazePch.h"

#include "Parse.h"
#include "HazeTokenText.h"
#include "HazeDebugDefine.h"
#include "HazeLogDefine.h"
#include "HazeFilePathHelper.h"

#include "Compiler.h"
#include "CompilerHelper.h"
#include "CompilerModule.h"
#include "CompilerValue.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerPointerFunction.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "CompilerFunction.h"
#include "CompilerBlock.h"
#include "CompilerClass.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"

struct PushTempRegister
{
	PushTempRegister(HAZE_STRING_STREAM& hss, Compiler* compiler, CompilerModule* compilerModule)
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
	Compiler* Compiler;
	CompilerModule* Module;
	HashMap<const HChar*, Share<CompilerValue>> UseTempRegisters;
};


CompilerModule::CompilerModule(Compiler* compiler, const HString& moduleName, const HString& modulePath)
	: m_Compiler(compiler), m_ModuleLibraryType(HazeLibraryType::Normal), m_IsBeginCreateFunctionVariable(false),
	 m_IsGenTemplateCode(false), m_Path(modulePath)
{
#if HAZE_I_CODE_ENABLE

	m_FS_I_Code.imbue(std::locale("chs"));
	m_FS_I_Code.open(GetIntermediateModuleFile(moduleName));

#endif
}

CompilerModule::~CompilerModule()
{
	if (m_FS_I_Code.is_open())
	{
		m_FS_I_Code.close();
	}
}

const HString& CompilerModule::GetName() const
{
	return *m_Compiler->GetModuleName(this);
}

void CompilerModule::MarkLibraryType(HazeLibraryType type)
{
	m_ModuleLibraryType = type;
}

void CompilerModule::RestartTemplateModule(const HString& moduleName)
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

void CompilerModule::FinishModule()
{
	m_CurrFunction.clear();
}

void CompilerModule::GenCodeFile()
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

Share<CompilerClass> CompilerModule::CreateClass(const HString& name, V_Array<CompilerClass*>& parentClass,
	V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<CompilerValue>>>>>& classData)
{
	auto compilerClass = GetClass(name);
	if (!compilerClass)
	{
		m_HashMap_Classes[name] = MakeShare<CompilerClass>(this, name, parentClass, classData);
		compilerClass = m_HashMap_Classes[name];

		m_Compiler->OnCreateClass(compilerClass);
		compilerClass->InitThisValue();
	}

	m_CurrClass = name;
	return compilerClass;
}

Share<CompilerEnum> CompilerModule::CreateEnum(const HString& name, HazeValueType baseType)
{
	Share<CompilerEnum> compilerEnum = GetEnum(this, name);
	if (!compilerEnum)
	{
		compilerEnum = MakeShare<CompilerEnum>(this, name, baseType);
		m_HashMap_Enums[name] = compilerEnum;
	
		m_CurrEnum = name;
	}

	return compilerEnum;
}

Share<CompilerEnum> CompilerModule::GetCurrEnum()
{
	auto iter = m_HashMap_Enums.find(m_CurrEnum);
	if (iter != m_HashMap_Enums.end())
	{
		return iter->second;
	}

	return nullptr;
}

void CompilerModule::FinishCreateEnum()
{
	m_CurrEnum.clear();
}

void CompilerModule::FinishCreateClass()
{
	m_CurrClass.clear();
}

Share<CompilerFunction> CompilerModule::GetCurrFunction()
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

Pair<Share<CompilerFunction>, Share<CompilerValue>> CompilerModule::GetFunction(const HString& name)
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

void CompilerModule::StartCacheTemplate(HString& templateName, uint32 startLine, HString& templateText, V_Array<HString>& templateTypes)
{
	auto iter = m_HashMap_TemplateText.find(templateName);
	if (iter == m_HashMap_TemplateText.end())
	{
		m_HashMap_TemplateText.insert({ Move(templateName), { startLine, Move(templateText),
			Move(templateTypes) } });
	}
}

bool CompilerModule::IsTemplateClass(const HString& name)
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

bool CompilerModule::ResetTemplateClassRealName(HString& inName, const V_Array<HazeDefineType>& templateTypes)
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

Share<CompilerEnum> CompilerModule::GetEnum(CompilerModule* m, const HString& name)
{
	auto ret = m->GetEnum_Internal(name);
	if (ret)
	{
		return ret;
	}

	return m->m_Compiler->GetBaseModuleEnum(name);
}

Share<CompilerFunction> CompilerModule::CreateFunction(const HString& name, HazeDefineType& type, V_Array<HazeDefineVariable>& params)
{
	Share<CompilerFunction> function = nullptr;
	auto it = m_HashMap_Functions.find(name);
	if (it == m_HashMap_Functions.end())
	{
		m_HashMap_Functions[name] = MakeShare<CompilerFunction>(this, name, type, params);
		m_HashMap_Functions[name]->InitEntryBlock(CompilerBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, m_HashMap_Functions[name], nullptr));

		function = m_HashMap_Functions[name];
	}
	else
	{
		function = it->second;
	}

	m_CurrFunction = name;
	return function;
}

Share<CompilerFunction> CompilerModule::CreateFunction(Share<CompilerClass> compilerClass, const HString& name,
	HazeDefineType& type, V_Array<HazeDefineVariable>& params)
{
	Share<CompilerFunction> function = compilerClass->FindFunction(name);
	if (!function)
	{
		function = MakeShare<CompilerFunction>(this, GetHazeClassFunctionName(compilerClass->GetName(), name), type, params, compilerClass.get());
		compilerClass->AddFunction(function);

		function->InitEntryBlock(CompilerBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, function, nullptr));
	}

	m_CurrFunction = name;
	return function;
}

void CompilerModule::FinishFunction()
{
	GetCurrFunction()->FunctionFinish();
	m_CurrFunction.clear();
}

void CompilerModule::CreateAdd(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::ADD);
}

void CompilerModule::CreateSub(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::SUB);
}

void CompilerModule::CreateMul(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::MUL);
}

void CompilerModule::CreateDiv(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::DIV);
}

void CompilerModule::CreateMod(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::MOD);
}

void CompilerModule::CreateBitAnd(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::BIT_AND);
}

void CompilerModule::CreateBitOr(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::BIT_OR);
}

void CompilerModule::CreateBitNeg(Share<CompilerValue> assignTo, Share<CompilerValue> value)
{
	GenIRCode_UnaryOperator(value, InstructionOpCode::BIT_NEG);
}

void CompilerModule::CreateBitXor(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::BIT_XOR);
}

void CompilerModule::CreateShl(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::SHL);
}

void CompilerModule::CreateShr(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::SHR);
}

Share<CompilerValue> CompilerModule::CreateNot(Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(nullptr, left, right, InstructionOpCode::NOT);
	return left;
}

void CompilerModule::CreateNeg(Share<CompilerValue> assignTo, Share<CompilerValue> value)
{
	if (value->IsConstant())
	{
		value = m_Compiler->GenConstantValue(value->GetValueType().PrimaryType, GetNegValue(value->GetValueType().PrimaryType, value->GetValue()));
	}
	else
	{
		auto tempRegister = m_Compiler->GetTempRegister(value->GetValueType());
		m_Compiler->CreateMov(tempRegister, value);

		HAZE_STRING_STREAM ss;
		GenIRCode(ss, this, InstructionOpCode::NEG, nullptr, tempRegister);
	
		Share<CompilerBlock> BB = m_Compiler->GetInsertBlock();
		BB->PushIRCode(ss.str());

		value = tempRegister;
	}
}

Share<CompilerValue> CompilerModule::CreateInc(Share<CompilerValue> value, bool isPreInc)
{
	Share<CompilerValue> retValue = value;
	if (IsHazeBaseType(value->GetValueType().PrimaryType))
	{
		if (!isPreInc)
		{
			retValue = m_Compiler->CreateMov(m_Compiler->GetTempRegister(value->GetValueType()), value);
		}
		//Compiler->CreateMov(Value, CreateAdd(Value, Compiler->GetConstantValueInt(1)));
		m_Compiler->CreateAdd(value, value, m_Compiler->GetConstantValueInt(1));
	}
	else
	{
		HAZE_LOG_ERR_W("<%s>类型不能使用Inc操作\n", GetHazeValueTypeString(value->GetValueType().PrimaryType));
	}
	return retValue;
}

Share<CompilerValue> CompilerModule::CreateDec(Share<CompilerValue> value, bool isPreDec)
{
	Share<CompilerValue> retValue = value;
	if (IsHazeBaseType(value->GetValueType().PrimaryType))
	{
		if (!isPreDec)
		{
			retValue = m_Compiler->GetTempRegister(value->GetValueType());
			m_Compiler->CreateMov(retValue, value);
		}
		//Compiler->CreateMov(Value, CreateSub(Value, Compiler->GetConstantValueInt(1)));
		m_Compiler->CreateSub(value, value, m_Compiler->GetConstantValueInt(1));
	}
	else
	{
		HAZE_LOG_ERR_W("<%s>类型不能使用Dec操作\n", GetHazeValueTypeString(value->GetValueType().PrimaryType));
	}
	return retValue;
}

void CompilerModule::GenIRCode_BinaryOperater(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2,
	InstructionOpCode opCode)
{
	//Share<HazeCompilerValue> retValue = left;

	//优化相关
	//if (oper1->IsConstant() && oper2->IsConstant())
	//{
	//	if (IsNumberType(oper1->GetValueType().PrimaryType))
	//	{
	//		auto& leftValue = const_cast<HazeValue&>(oper1->GetValue());
	//		HazeValue tempValue = leftValue;
	//		auto& rightValue = const_cast<HazeValue&>(oper2->GetValue());
	//		CalculateValueByType(oper1->GetValueType().PrimaryType, opCode, &rightValue, &leftValue);

	//		auto retValue = m_Compiler->GenConstantValue(oper1->GetValueType().PrimaryType, leftValue);
	//		leftValue = tempValue;
	//	}
	//	else
	//	{
	//		COMPILER_ERR_MODULE_W("生成<%s>操作错误", GetInstructionString(opCode), GetName().c_str());
	//	}
	//	return;// retValue;
	//}

	HAZE_STRING_STREAM ss;
	
	GenIRCode(ss, this, opCode, assignTo, oper1, oper2);
	if (!m_CurrFunction.empty())
	{
		m_Compiler->GetInsertBlock()->PushIRCode(ss.str());
	}
	else
	{
		m_ModuleIRCodes.push_back(ss.str());
	}

	//return retValue;
}

void CompilerModule::GenIRCode_UnaryOperator(Share<CompilerValue> value, InstructionOpCode opCode)
{
	auto function = GetCurrFunction();
	HAZE_STRING_STREAM hss;

	if (m_CurrFunction.empty())
	{
		HAZE_TO_DO(全局语句暂时不处理);
	}
	else
	{
		GenIRCode(hss, this, opCode, nullptr, value);
	}

	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void CompilerModule::GenIRCode_Ret(Share<CompilerValue> value)
{
	auto function = GetCurrFunction();
	HAZE_STRING_STREAM hss;

	GenIRCode(hss, this, InstructionOpCode::RET, nullptr, value);
	hss << std::endl;
	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void CompilerModule::GenIRCode_Cmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock)
{
	HAZE_STRING_STREAM hss;

	if (cmpType == HazeCmpType::None)
	{
		HAZE_LOG_ERR_W("比较失败,比较类型为空,当前函数<%s>!\n", GetCurrFunction()->GetName().c_str());
	}

	GenIRCode(hss, this, GetInstructionOpCodeByCmpType(cmpType), ifJmpBlock, elseJmpBlock);
	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void CompilerModule::GenIRCode_JmpTo(Share<CompilerBlock> block)
{
	HAZE_STRING_STREAM hss;
	GenIRCode(hss, this, InstructionOpCode::JMP, block);
	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

Share<CompilerValue> CompilerModule::CreateGlobalVariable(const HazeDefineVariable& var, Share<CompilerValue> refValue,
	uint64 arrayDimension, V_Array<HazeDefineType>* params)
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
		CreateVariable(this, var.Type, HazeVariableScope::Global, HazeDataDesc::None, 0, refValue, arrayDimension, params) });

	auto& retValue = m_Variables.back().second;
	if (!IsHazeBaseType(retValue->GetValueType().PrimaryType))
	{
		if (retValue->IsClass())
		{

		}
	}



	return retValue;
}

void CompilerModule::FunctionCall(HAZE_STRING_STREAM& hss, Share<CompilerFunction> callFunction, Share<CompilerValue> pointerFunction,
	AdvanceFunctionInfo* advancFunctionInfo, uint32& size, V_Array<Share<CompilerValue>>& params,
	Share<CompilerValue> thisPointerTo)
{
	Share<CompilerBlock> insertBlock = m_Compiler->GetInsertBlock();
	HString strName;

	auto pointerFunc = DynamicCast<CompilerPointerFunction>(pointerFunction);

	V_Array<const HazeDefineType*> funcTypes(params.size());
	if (!callFunction && !pointerFunc && !advancFunctionInfo)
	{
		COMPILER_ERR_MODULE_W("生成函数调用错误, <%s>为空", GetName().c_str(),
			callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"));
	}
	else
	{
		auto paramSize = callFunction ? callFunction->m_Params.size() : pointerFunc ? pointerFunc->m_ParamTypes.size() :
			advancFunctionInfo->Params.size();
		for (int64 i = params.size() - 1; i >= 0; i--)
		{
			auto Variable = params[i];
			auto type = callFunction ? &callFunction->GetParamTypeLeftToRightByIndex(params.size() - 1 - i) :
				pointerFunc ? &pointerFunc->GetParamTypeLeftToRightByIndex(params.size() - 1 - i) :
				advancFunctionInfo->Params.size() > params.size() - 1 - i ? &advancFunctionInfo->Params.at(params.size() - 1 - i)
				: &advancFunctionInfo->Params.at(advancFunctionInfo->Params.size() - 1);

			if (*type != Variable->GetValueType() && !Variable->GetValueType().IsStrongerType(*type))
			{
				if (i == (int64)params.size() - 1 && !IsMultiVariableTye(type->PrimaryType) && paramSize - (callFunction && callFunction->GetClass() ? 1 : 0) != params.size())
				{
					COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 应填入<%d>个参数，实际填入了<%d>个", GetName().c_str(),
						callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"), paramSize, params.size());
				}
				else if (IsMultiVariableTye(type->PrimaryType) && i == 0) {}
				else if (Variable->IsEnum())
				{
					auto enumValue = DynamicCast<CompilerEnumValue>(Variable);
					if (enumValue && enumValue->GetEnum() && enumValue->GetEnum()->GetParentType() == type->PrimaryType) {}
					else
					{
						COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数枚举类型不匹配", GetName().c_str(),
							callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"), params.size() - 1 - i);
					}
				}
				else if (type->IsStrongerType(Variable->GetValueType())) {}
				else if (IsRefrenceType(type->PrimaryType) && Variable->GetValueType().PrimaryType == type->SecondaryType) {}
				else if (!IsMultiVariableTye(type->PrimaryType))
				{
					COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数类型不匹配", GetName().c_str(),
						callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"),
						params.size() - 1 - i);
				}
			}

			funcTypes[i] = IsMultiVariableTye(type->PrimaryType) ? &Variable->GetValueType() : type;
		}
	}

	for (uint64 i = 0; i < params.size(); i++)
	{
		GenIRCode(hss, this, InstructionOpCode::PUSH, nullptr, params[i], nullptr, funcTypes[i]);

		if (insertBlock)
		{
			insertBlock->PushIRCode(hss.str());
		}
		else
		{
			m_ModuleIRCodes.push_back(hss.str());
		}

		hss.str(H_TEXT(""));

		size += GetSizeByCompilerValue(params[i]);
		strName.clear();
	}

	if (thisPointerTo)
	{
		GenIRCode(hss, this, InstructionOpCode::PUSH, nullptr, thisPointerTo);

		if (insertBlock)
		{
			insertBlock->PushIRCode(hss.str());
		}
		else
		{
			m_ModuleIRCodes.push_back(hss.str());
		}

		hss.str(H_TEXT(""));

		size += GetSizeByCompilerValue(thisPointerTo);
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

Share<CompilerValue> CompilerModule::CreateFunctionCall(Share<CompilerFunction> callFunction, 
	V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo)
{
	HAZE_STRING_STREAM hss;
	uint32 size = 0;

	PushTempRegister pushTempRegister(hss, m_Compiler, this);
	FunctionCall(hss, callFunction, nullptr, nullptr, size, params, thisPointerTo);

	GenIRCode(hss, this, InstructionOpCode::CALL, params.size(), size, callFunction);
	/*hss << GetInstructionString(InstructionOpCode::CALL) << " " << callFunction->GetName() << " " << CAST_TYPE(HazeValueType::None) 
		<< " " << CAST_SCOPE(HazeVariableScope::Ignore) << " " << CAST_DESC(HazeDataDesc::FunctionAddress)
		<< " " << params.size() << " " << size << " " << callFunction->GetModule()->GetName() << " "
		<< CAST_DESC(HazeDataDesc::CallFunctionModule) << std::endl;*/

	auto retRegister = Compiler::GetRegister(RET_REGISTER);

	auto& retRegisterType = const_cast<HazeDefineType&>(retRegister->GetValueType());
	retRegisterType = callFunction->GetFunctionType();
	return retRegister;
}

Share<CompilerValue> CompilerModule::CreateFunctionCall(Share<CompilerValue> pointerFunction, 
	V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo)
{
	Share<CompilerBlock> insertBlock = m_Compiler->GetInsertBlock();
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
	GenIRCode(hss, this, InstructionOpCode::CALL, params.size(), size, nullptr, pointerFunction);
	/*hss << GetInstructionString(InstructionOpCode::CALL) << " " << varName << " " << CAST_TYPE(HazeValueType::Function) << " "
		<< CAST_SCOPE(pointerFunction->GetVariableScope())  << " " << CAST_DESC(pointerFunction->GetVariableDesc()) << " " << params.size()
		<< " " << size << " " << m_Compiler->GetCurrModuleName() << " " << CAST_DESC(HazeDataDesc::CallFunctionModule) << std::endl;*/

	return Compiler::GetRegister(RET_REGISTER);
}

Share<CompilerValue> CompilerModule::CreateAdvanceTypeFunctionCall(AdvanceFunctionInfo& functionInfo, 
	V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo)
{
	Share<CompilerBlock> insertBlock = m_Compiler->GetInsertBlock();
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
	GenIRCode(hss, this, InstructionOpCode::CALL, params.size(), size, nullptr, nullptr, thisPointerTo, functionInfo.ClassFunc);
	/*hss << GetInstructionString(InstructionOpCode::CALL) << " " << varName << " " << CAST_TYPE(HazeValueType::Function) << " "
		<< CAST_SCOPE(thisPointerTo->GetVariableScope()) << " " << CAST_DESC(thisPointerTo->GetVariableDesc()) << " " << params.size()
		<< " " << size << " " << m_Compiler->GetCurrModuleName() << " " << CAST_DESC(HazeDataDesc::CallFunctionPointer)
		<< " " << functionInfo.ClassFunc << std::endl;*/

	return Compiler::GetRegister(RET_REGISTER);
}

Share<CompilerValue> CompilerModule::GetOrCreateGlobalStringVariable(const HString& str)
{
	auto it = m_HashMap_StringTable.find(str);
	if (it != m_HashMap_StringTable.end())
	{
		return it->second;
	}
	m_HashMap_StringTable[str] = nullptr;

	it = m_HashMap_StringTable.find(str);

	m_HashMap_StringMapping[(int)m_HashMap_StringMapping.size()] = &it->first;

	HazeDefineType defineVarType;
	defineVarType.PrimaryType = HazeValueType::String;

	it->second = CreateVariable(this, defineVarType, HazeVariableScope::Global, HazeDataDesc::ConstantString, 0);

	HazeValue& hazeValue = const_cast<HazeValue&>(it->second->GetValue());
	hazeValue.Value.Extra.StringTableIndex = (int)m_HashMap_StringMapping.size() - 1;

	return it->second;
}

uint32 CompilerModule::GetGlobalStringIndex(Share<CompilerValue> value)
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

void CompilerModule::PreStartCreateGlobalVariable()
{
	m_VariablesAddress.push_back({ (int)m_ModuleIRCodes.size(), 0 });
}

void CompilerModule::EndCreateGlobalVariable()
{
	m_VariablesAddress[m_VariablesAddress.size() - 1].second = (int)m_ModuleIRCodes.size();
}

Share<CompilerValue> CompilerModule::GetGlobalVariable(CompilerModule* m, const HString& name)
{
	auto ret = m->GetGlobalVariable_Internal(name);
	if (ret)
	{
		return ret;
	}

	return m->m_Compiler->GetBaseModuleGlobalVariable(name);
}

bool CompilerModule::GetGlobalVariableName(CompilerModule* m, const Share<CompilerValue>& value, HString& outName, bool getOffset,
	V_Array<Pair<uint64, CompilerValue*>>* offsets)
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

Share<CompilerValue> CompilerModule::GetGlobalVariable_Internal(const HString& name)
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

bool CompilerModule::GetGlobalVariableName_Internal(const Share<CompilerValue>& value, HString& outName, bool getOffset,
	V_Array<Pair<uint64, CompilerValue*>>* offsets)
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

Share<CompilerEnum> CompilerModule::GetEnum_Internal(const HString& name)
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

Share<CompilerClass> CompilerModule::GetClass(const HString& className)
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

uint32 CompilerModule::GetClassSize(const HString& className)
{
	return GetClass(className)->GetDataSize();
}

void CompilerModule::GenICode()
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