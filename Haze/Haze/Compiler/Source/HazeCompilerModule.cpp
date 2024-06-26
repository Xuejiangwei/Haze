#include <filesystem>
#include <unordered_set>

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
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerInitlistValue.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerEnum.h"

struct PushTempRegister
{
	PushTempRegister(HAZE_STRING_STREAM& hss, HazeCompiler* compiler, HazeCompilerModule* compilerModule)
		: Size(0), Hss(hss), Compiler(compiler), Module(compilerModule)
	{
		UseTempRegisters = Compiler->GetUseTempRegister();
		for (auto& regi : UseTempRegisters)
		{
			Hss << GetInstructionString(InstructionOpCode::PUSH) << " ";
			Module->GenVariableHzic(Module, Hss, regi.second);
			Hss << std::endl;
			Size += regi.second->GetSize();
		}
	}

	~PushTempRegister()
	{
		for (auto& regi : UseTempRegisters)
		{
			Hss << GetInstructionString(InstructionOpCode::POP) << " ";
			Module->GenVariableHzic(Module, Hss, regi.second);
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

	int Size;
private:
	HAZE_STRING_STREAM& Hss;
	HazeCompiler* Compiler;
	HazeCompilerModule* Module;
	std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> UseTempRegisters;
};


HazeCompilerModule::HazeCompilerModule(HazeCompiler* compiler, const HAZE_STRING& moduleName)
	: m_Compiler(compiler), m_ModuleLibraryType(HazeLibraryType::Normal), m_IsBeginCreateFunctionVariable(false),
	 m_IsGenTemplateCode(false)
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

const HAZE_STRING& HazeCompilerModule::GetName() const
{
	return *m_Compiler->GetModuleName(this);
}

void HazeCompilerModule::MarkLibraryType(HazeLibraryType type)
{
	m_ModuleLibraryType = type;
}

void HazeCompilerModule::RestartTemplateModule(const HAZE_STRING& moduleName)
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
	GenCodeFile();
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

std::shared_ptr<HazeCompilerClass> HazeCompilerModule::CreateClass(const HAZE_STRING& name, std::vector<HazeCompilerClass*>& parentClass,
	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& classData)
{
	auto compilerClass = GetClass(name);
	if (!compilerClass)
	{
		m_HashMap_Classes[name] = std::make_shared<HazeCompilerClass>(this, name, parentClass, classData);
		compilerClass = m_HashMap_Classes[name];
		compilerClass->InitThisValue();
	}

	m_CurrClass = name;
	return compilerClass;
}

std::shared_ptr<HazeCompilerEnum> HazeCompilerModule::CreateEnum(const HAZE_STRING& name, HazeValueType parentType, 
	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& enumValues)
{
	std::shared_ptr<HazeCompilerEnum> compilerEnum = GetEnum(name);
	if (!compilerEnum)
	{
		compilerEnum = std::make_shared<HazeCompilerEnum>(this, parentType);
		m_HashMap_Enums[name] = compilerEnum;
		compilerEnum->InitEnumValues(enumValues);
	}

	return compilerEnum;
}

void HazeCompilerModule::FinishCreateClass()
{
	m_CurrClass.clear();
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::GetCurrFunction()
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

std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> HazeCompilerModule::GetFunction(const HAZE_STRING& name)
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

void HazeCompilerModule::StartCacheTemplate(HAZE_STRING& templateName, uint32 startLine, HAZE_STRING& templateText, std::vector<HAZE_STRING>& templateTypes)
{
	auto iter = m_HashMap_TemplateText.find(templateName);
	if (iter == m_HashMap_TemplateText.end())
	{
		m_HashMap_TemplateText.insert({ std::move(templateName), { startLine, std::move(templateText), std::move(templateTypes) } });
	}
}

bool HazeCompilerModule::IsTemplateClass(const HAZE_STRING& name)
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

bool HazeCompilerModule::ResetTemplateClassRealName(HAZE_STRING& inName, const std::vector<HazeDefineType>& templateTypes)
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

			//HAZE_STRING className = inName;
			GetTemplateClassName(inName, templateTypes);

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

std::shared_ptr<HazeCompilerEnum> HazeCompilerModule::GetEnum(const HAZE_STRING& name)
{
	auto iter = m_HashMap_Enums.find(name);
	if (iter != m_HashMap_Enums.end())
	{
		return iter->second;
	}

	for (auto& it : m_ImportModules)
	{
		auto ret = it->GetEnum(name);
		if (ret)
		{
			return ret;
		}
	}

	return m_Compiler->GetBaseModuleEnum(name);
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(const HAZE_STRING& name, HazeDefineType& type, std::vector<HazeDefineVariable>& params)
{
	std::shared_ptr<HazeCompilerFunction> function = nullptr;
	auto it = m_HashMap_Functions.find(name);
	if (it == m_HashMap_Functions.end())
	{
		m_HashMap_Functions[name] = std::make_shared<HazeCompilerFunction>(this, name, type, params);
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

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(std::shared_ptr<HazeCompilerClass> compilerClass, const HAZE_STRING& name,
	HazeDefineType& type, std::vector<HazeDefineVariable>& params)
{
	std::shared_ptr<HazeCompilerFunction> function = compilerClass->FindFunction(name);
	if (!function)
	{
		function = std::make_shared<HazeCompilerFunction>(this, GetHazeClassFunctionName(compilerClass->GetName(), name), type, params, compilerClass.get());
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

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateAdd(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::ADD_ASSIGN : InstructionOpCode::ADD);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateSub(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::SUB_ASSIGN : InstructionOpCode::SUB);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateMul(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::MUL_ASSIGN : InstructionOpCode::MUL);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateDiv(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::DIV_ASSIGN : InstructionOpCode::DIV);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateMod(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::MOD_ASSIGN : InstructionOpCode::MOD);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateBitAnd(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::BIT_AND_ASSIGN : InstructionOpCode::BIT_AND);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateBitOr(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::BIT_OR_ASSIGN : InstructionOpCode::BIT_OR);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateBitNeg(std::shared_ptr<HazeCompilerValue> value)
{
	return GenIRCode_BinaryOperater(value, nullptr, InstructionOpCode::BIT_NEG);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateBitXor(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::BIT_XOR_ASSIGN : InstructionOpCode::BIT_XOR);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateShl(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::SHL_ASSIGN : InstructionOpCode::SHL);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateShr(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GenIRCode_BinaryOperater(left, right, isAssign ? InstructionOpCode::SHR_ASSIGN : InstructionOpCode::SHR);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateNot(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right)
{
	return GenIRCode_BinaryOperater(left, right, InstructionOpCode::NOT);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateNeg(std::shared_ptr<HazeCompilerValue> value)
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

		std::shared_ptr<HazeBaseBlock> BB = m_Compiler->GetInsertBlock();
		BB->PushIRCode(ss.str());

		value = tempRegister;
	}

	return value;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateInc(std::shared_ptr<HazeCompilerValue> value, bool isPreInc)
{
	std::shared_ptr<HazeCompilerValue> retValue = value;
	if (IsHazeDefaultType(value->GetValueType().PrimaryType) || value->IsPointer())
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
		HAZE_LOG_ERR(HAZE_TEXT("<%s>类型不能使用Inc操作\n"), GetHazeValueTypeString(value->GetValueType().PrimaryType));
	}
	return retValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateDec(std::shared_ptr<HazeCompilerValue> value, bool isPreDec)
{
	std::shared_ptr<HazeCompilerValue> retValue = value;
	if (IsHazeDefaultType(value->GetValueType().PrimaryType) || value->IsPointer())
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
		HAZE_LOG_ERR(HAZE_TEXT("<%s>类型不能使用Dec操作\n"), GetHazeValueTypeString(value->GetValueType().PrimaryType));
	}
	return retValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateArrayInit(std::shared_ptr<HazeCompilerValue> array, std::shared_ptr<HazeCompilerValue> initList)
{
	auto arrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(array);
	auto initListValue = std::dynamic_pointer_cast<HazeCompilerInitListValue>(initList);

	HAZE_STRING_STREAM hss;
	HAZE_STRING varName;

	if (IsHazeDefaultType(arrayValue->GetValueType().SecondaryType))
	{
		for (int i = 0; i < (int)arrayValue->GetArrayLength(); i++)
		{
			hss << GetInstructionString(InstructionOpCode::MOV) << " ";

			GenVariableHzic(this, hss, array, i);

			hss << " ";

			GenVariableHzic(this, hss, initListValue->GetList()[i]);

			hss << std::endl;

			std::shared_ptr<HazeBaseBlock> BB = m_Compiler->GetInsertBlock();
			BB->PushIRCode(hss.str());
			hss.str(HAZE_TEXT(""));
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("目前只支持Haze默认类型的数组初始化赋值!\n"));
	}

	return array;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GenIRCode_BinaryOperater(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, InstructionOpCode opCode)
{
	static std::unordered_set<InstructionOpCode> s_HashSet_NoTemp =
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

	std::shared_ptr<HazeCompilerValue> retValue = left;

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

		ss << GetInstructionString(opCode) << " ";
		GenVariableHzic(this, ss, retValue);

		if (right)
		{
			ss << " ";
			GenVariableHzic(this, ss, right);
		}

		ss << std::endl;

		m_Compiler->GetInsertBlock()->PushIRCode(ss.str());
	}
	else if (left->IsGlobalVariable())
	{
		ss << GetInstructionString(opCode) << " ";
		GenVariableHzic(this, ss, retValue);

		if (right)
		{
			ss << " ";
			GenVariableHzic(this, ss, right);
		}

		ss << std::endl;

		m_ModuleIRCodes.push_back(ss.str());
	}

	return retValue;
}

void HazeCompilerModule::GenIRCode_UnaryOperator(std::shared_ptr<HazeCompilerValue> value, InstructionOpCode opCode)
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

void HazeCompilerModule::GenIRCode_Ret(std::shared_ptr<HazeCompilerValue> value)
{
	auto function = GetCurrFunction();
	HAZE_STRING_STREAM hss;
	hss << GetInstructionString(InstructionOpCode::RET) << " ";
	GenVariableHzic(this, hss, value);

	hss << std::endl;

	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void HazeCompilerModule::GenIRCode_Cmp(HazeCmpType cmpType, std::shared_ptr<HazeBaseBlock> ifJmpBlock, std::shared_ptr<HazeBaseBlock> elseJmpBlock)
{
	HAZE_STRING_STREAM hss;

	if (cmpType == HazeCmpType::None)
	{
		HAZE_LOG_ERR(HAZE_TEXT("比较失败,比较类型为空,当前函数<%s>!\n"), GetCurrFunction()->GetName().c_str());
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

void HazeCompilerModule::GenIRCode_JmpTo(std::shared_ptr<HazeBaseBlock> block)
{
	HAZE_STRING_STREAM hss;
	hss << GetInstructionString(InstructionOpCode::JMP) << " " << block->GetName() << std::endl;

	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateGlobalVariable(const HazeDefineVariable& var, std::shared_ptr<HazeCompilerValue> refValue,
	std::vector<std::shared_ptr<HazeCompilerValue>> arraySize, std::vector<HazeDefineType>* params)
{
	for (auto& it : m_Variables)
	{
		if (it.first == var.Name)
		{
			HAZE_LOG_ERR(HAZE_TEXT("编译器错误,添加全局变量重复\n"));
			return nullptr;
		}
	}

	m_Variables.push_back({ var.Name,
		CreateVariable(this, var, HazeVariableScope::Global, HazeDataDesc::None, 0, refValue, arraySize, params) });

	auto& retValue = m_Variables.back().second;
	if (!IsHazeDefaultType(retValue->GetValueType().PrimaryType))
	{
		if (retValue->IsClass())
		{

		}
	}



	return retValue;
}

void HazeCompilerModule::FunctionCall(HAZE_STRING_STREAM& hss, const HAZE_STRING& callName, uint32& size, 
	std::vector<std::shared_ptr<HazeCompilerValue>>& params, std::shared_ptr<HazeCompilerValue> thisPointerTo)
{
	std::shared_ptr<HazeBaseBlock> insertBlock = m_Compiler->GetInsertBlock();
	HAZE_STRING strName;

	for (size_t i = 0; i < params.size(); i++)
	{
		auto Variable = params[i];
		if (params[i]->IsRef())
		{
			Variable = m_Compiler->CreateMovPV(m_Compiler->GetTempRegister(), params[i]);
		}
		else if (params[i]->IsArrayElement())
		{
			Variable = GetArrayElementToValue(this, params[i], m_Compiler->GetTempRegister());
		}

		hss << GetInstructionString(InstructionOpCode::PUSH) << " ";
		GenVariableHzic(this, hss, Variable);

		hss << std::endl;

		if (insertBlock)
		{
			insertBlock->PushIRCode(hss.str());
		}
		else
		{
			m_ModuleIRCodes.push_back(hss.str());
		}

		hss.str(HAZE_TEXT(""));

		size += Variable->GetSize();
		strName.clear();
	}

	if (thisPointerTo)
	{
		if (GetCurrFunction())
		{
			if (!GetCurrFunction()->FindLocalVariableName(thisPointerTo, strName))
			{
				if (!GetGlobalVariableName(this, thisPointerTo, strName))
				{
					COMPILER_ERR_MODULE_W("生成函数<%s>调用错误,没有找到调用类对象变量", GetName().c_str(), callName.c_str());
				}
			}
		}
		else
		{
			if (!GetGlobalVariableName(this, thisPointerTo, strName))
			{
				COMPILER_ERR_MODULE_W("生成函数<%s>调用错误,没有找到调用类对象全局变量", GetName().c_str(), callName.c_str());
			}
		}

		hss << GetInstructionString(InstructionOpCode::PUSH) << " ";
		if (thisPointerTo->IsPointerClass())
		{

			//auto pointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(thisPointerTo);
			hss << strName << " " << CAST_SCOPE(thisPointerTo->GetVariableScope()) << " " << CAST_DESC(thisPointerTo->GetVariableDesc()) << " " <<//CAST_DESC(HazeDataDesc::ClassPointer) << " " <<
				CAST_TYPE(HazeValueType::PointerClass) << " " << thisPointerTo->GetValueType().CustomName;
		}
		else if (thisPointerTo->IsClass())
		{
			auto classValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(thisPointerTo);
			hss << strName << " " << CAST_SCOPE(classValue->GetVariableScope()) << " " << CAST_TYPE(HazeDataDesc::ClassThis) << " " <<
				CAST_TYPE(HazeValueType::PointerClass) << " " << classValue->GetOwnerClassName();
		}
		else if (thisPointerTo->IsRefClass())
		{
			hss << strName << " " << CAST_SCOPE(thisPointerTo->GetVariableScope()) << " " << CAST_DESC(thisPointerTo->GetVariableDesc()) << " " <<//CAST_DESC(HazeDataDesc::ClassPointer) << " " <<
				CAST_TYPE(HazeValueType::ReferenceClass) << " " << thisPointerTo->GetValueType().CustomName;
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("函数调用失败，己指针类型错误!\n"));
		}

		hss << std::endl;

		if (insertBlock)
		{
			insertBlock->PushIRCode(hss.str());
		}
		else
		{
			m_ModuleIRCodes.push_back(hss.str());
		}

		hss.str(HAZE_TEXT(""));

		size += GetSizeByHazeType(HazeValueType::PointerClass);
	}

	hss << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CALL_PUSH_ADDRESS_NAME << " " << CAST_SCOPE(HazeVariableScope::None)
		<< " " << (uint32)HazeDataDesc::Address << " " << CAST_TYPE(HazeValueType::Int) << std::endl;
	if (insertBlock)
	{
		insertBlock->PushIRCode(hss.str());
	}
	else
	{
		m_ModuleIRCodes.push_back(hss.str());
	}
	hss.str(HAZE_TEXT(""));
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> callFunction, 
	std::vector<std::shared_ptr<HazeCompilerValue>>& params, std::shared_ptr<HazeCompilerValue> thisPointerTo)
{
	HAZE_STRING_STREAM hss;
	uint32 size = 0;

	PushTempRegister pushTempRegister(hss, m_Compiler, this);
	FunctionCall(hss, callFunction->GetName(), size, params, thisPointerTo);

	hss << GetInstructionString(InstructionOpCode::CALL) << " " << callFunction->GetName() << " " << CAST_TYPE(HazeValueType::Function) 
		<< " " << params.size() << " " << size << " " << callFunction->GetModule()->GetName() << std::endl;

	auto retRegister = HazeCompiler::GetRegister(RET_REGISTER);

	auto& retRegisterType = const_cast<HazeDefineType&>(retRegister->GetValueType());
	retRegisterType = callFunction->GetFunctionType();
	return retRegister;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(std::shared_ptr<HazeCompilerValue> pointerFunction, 
	std::vector<std::shared_ptr<HazeCompilerValue>>& params, std::shared_ptr<HazeCompilerValue> thisPointerTo)
{
	std::shared_ptr<HazeBaseBlock> insertBlock = m_Compiler->GetInsertBlock();
	HAZE_STRING_STREAM hss;
	uint32 size = 0;

	HAZE_STRING varName;
	GetGlobalVariableName(this, pointerFunction, varName);
	if (varName.empty())
	{
		GetCurrFunction()->FindLocalVariableName(pointerFunction, varName);
		if (varName.empty())
		{
			HAZE_LOG_ERR(HAZE_TEXT("函数指针调用失败!\n"));
			return nullptr;
		}
	}

	PushTempRegister pushTempRegister(hss, m_Compiler, this);

	FunctionCall(hss, varName, size, params, thisPointerTo);

	hss << GetInstructionString(InstructionOpCode::CALL) << " " << varName << " " << CAST_TYPE(HazeValueType::PointerFunction) << " "
		<< CAST_SCOPE(pointerFunction->GetVariableScope())  << " " << CAST_DESC(pointerFunction->GetVariableDesc()) << " " << params.size()
		<< " " << size << " " << m_Compiler->GetCurrModuleName() << std::endl;
	insertBlock->PushIRCode(hss.str());

	return HazeCompiler::GetRegister(RET_REGISTER);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GetOrCreateGlobalStringVariable(const HAZE_STRING& str)
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
	defineVar.Type.PrimaryType = HazeValueType::PointerBase;
	defineVar.Type.SecondaryType = HazeValueType::Char;

	it->second = CreateVariable(this, defineVar, HazeVariableScope::Global, HazeDataDesc::ConstantString, 0);

	HazeValue& hazeValue = const_cast<HazeValue&>(it->second->GetValue());
	hazeValue.Value.Extra.StringTableIndex = (int)m_HashMap_StringMapping.size() - 1;

	return it->second;
}

uint32 HazeCompilerModule::GetGlobalStringIndex(std::shared_ptr<HazeCompilerValue> value)
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

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GetGlobalVariable(HazeCompilerModule* m, const HAZE_STRING& name)
{
	auto ret = m->GetGlobalVariable_Internal(name);
	if (ret)
	{
		return ret;
	}

	return m->m_Compiler->GetBaseModuleGlobalVariable(name);
}

bool HazeCompilerModule::GetGlobalVariableName(HazeCompilerModule* m, const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName)
{
	if (m->GetGlobalVariableName_Internal(value, outName))
	{
		return true;
	}

	if (m->m_Compiler->GetBaseModuleGlobalVariableName(value, outName))
	{
		return true;
	}

	return value->TryGetVariableName(outName);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GetGlobalVariable_Internal(const HAZE_STRING& name)
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

bool HazeCompilerModule::GetGlobalVariableName_Internal(const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName)
{
	if (value->IsRegister())
	{
		outName = m_Compiler->GetRegisterName(value);
		return true;
	}

	for (auto& it : m_Variables)
	{
		if (TrtGetVariableName(nullptr, it, value, outName))
		{
			return true;
		}
	}

	for (auto& it : m_ImportModules)
	{
		if (it->GetGlobalVariableName_Internal(value, outName))
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

std::shared_ptr<HazeCompilerClass> HazeCompilerModule::GetClass(const HAZE_STRING& className)
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

uint32 HazeCompilerModule::GetClassSize(const HAZE_STRING& className)
{
	return GetClass(className)->GetDataSize();
}

void HazeCompilerModule::GenVariableHzic(HazeCompilerModule* compilerModule, HAZE_STRING_STREAM& hss, 
	const std::shared_ptr<HazeCompilerValue>& value, int index)
{
	static HAZE_STRING s_StrName;

	bool find = false;

	s_StrName.clear();
	if (value->IsGlobalVariable())
	{
		find = HazeCompilerModule::GetGlobalVariableName(compilerModule, value, s_StrName);
		if (!find && value->IsNullPtr())
		{
			find = true;
			s_StrName = NULL_PTR;
		}
	}
	else if (value->IsLocalVariable())
	{
		find = compilerModule->GetCurrFunction()->FindLocalVariableName(value, s_StrName);
	}
	else if (value->IsTempVariable())
	{
		find = true;
		s_StrName = compilerModule->m_Compiler->GetRegisterName(value);
	}
	else if (value->IsFunctionAddress())
	{
		s_StrName = value->GetValueType().CustomName;
		if (!s_StrName.empty())
		{
			find = true;
		}
	}
	else
	{
		HAZE_LOG_ERR_W("生成中间代码错误,变量作用域错误!\n");
		return;
	}

	if (!find)
	{
		HAZE_LOG_ERR_W("生成中间代码错误,未能找到变量!\n");
		return;
	}

	hss << s_StrName << " " << CAST_SCOPE(value->GetVariableScope()) << " " << CAST_DESC(value->GetVariableDesc()) << " ";
	value->GetValueType().StringStreamTo(hss);

	if (value->IsString())
	{
		index = compilerModule->GetGlobalStringIndex(value);
	}

	if (index >= 0)
	{
		hss << " " << index;
	}
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

		hss << var.first << " " << var.second->GetSize() << " " << CAST_TYPE(var.second->GetValueType().PrimaryType) << " ";

		if (IsHazeDefaultType(var.second->GetValueType().PrimaryType))
		{
			HazeCompilerStream(hss, var.second);
		}
		else
		{
			if (var.second->GetValueType().NeedCustomName())
			{
				hss << var.second->GetValueType().CustomName;
			}
		}

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
		HAZE_LOG_ERR(HAZE_TEXT("生成字符串表失败!"),
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