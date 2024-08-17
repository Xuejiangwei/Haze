#include "HazePch.h"
#include "HazeVM.h"
#include "HazeLogDefine.h"
#include "HazeFilePathHelper.h"

#include "HazeCompiler.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerInitListValue.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerEnum.h"
#include "HazeCompilerEnumValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerHelper.h"

//static Share<HazeCompilerFunction> s_GlobalVariebleDefine = MakeShare<HazeCompilerFunction>();

static HashMap<const HChar*, Share<HazeCompilerValue>> g_GlobalRegisters = {
	{ RET_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("Ret_Register")), HazeVariableScope::Global, HazeDataDesc::RegisterRet, 0) },
	{ NEW_REGISTER, nullptr },
	{ CMP_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("Cmp_Register")), HazeVariableScope::Global, HazeDataDesc::RegisterCmp, 0) },
};

// ֻ��������ʱ�����Ĵ���, ÿ������ջ��Ҫ�洢����������ʱ�Ĵ���, GCʱҲҪɨ��
static HashMap<const HChar*, Share<HazeCompilerValue>> g_GlobalTempRegisters = {
	{ TEMP_REGISTER_A, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_B, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	/*{ TEMP_REGISTER_2, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_3, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_4, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_5, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_6, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_7, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_8, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_9, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },*/
};

static Share<HazeCompilerInitListValue> s_InitializeListValue = MakeShare<HazeCompilerInitListValue>(nullptr,
	HazeDefineType(HazeValueType::Void), HazeVariableScope::Temp, HazeDataDesc::Initlist, 0);

HazeCompiler::HazeCompiler(HazeVM* vm) : m_VM(vm)
{
	m_AdvanceClassInfo.clear();
}

HazeCompiler::~HazeCompiler()
{
}

void HazeCompiler::RegisterAdvanceClassInfo(HazeValueType type, AdvanceClassInfo& info)
{
	if (IsAdvanceType(type))
	{
		m_AdvanceClassInfo[type] = info;
	}
	else
	{
		HAZE_LOG_ERR_W("���������Ϣ����!\n");
	}
}

bool HazeCompiler::InitializeCompiler(const HString& moduleName, const HString& path)
{
	//����ģ��
	auto It = m_CompilerModules.find(moduleName);
	if (It != m_CompilerModules.end())
	{
		return false;
	}

	m_ModuleNameStack.push_back(moduleName);
	m_CompilerModules[moduleName] = MakeUnique<HazeCompilerModule>(this, moduleName, path);
	return true;
}

void HazeCompiler::FinishParse()
{
	HAZE_OFSTREAM ofstream;
	ofstream.imbue(std::locale("chs"));
	ofstream.open(GetIntermediateModuleFile(HAZE_INTER_SYMBOL_TABLE));

	HAZE_STRING_STREAM hss;

	hss << GetSymbolBeginHeader() << std::endl;
	for (auto& m : m_CompilerModules)
	{
		for (auto& className : m.second->m_HashMap_Classes)
		{
			hss << className.first << std::endl;
		}
	}
	hss << GetSymbolEndHeader() << std::endl;

	ofstream << hss.str();
	ofstream.close();


	for (auto& m : m_CompilerModules)
	{
		m.second->GenCodeFile();
	}
}

HazeCompilerModule* HazeCompiler::ParseBaseModule(const HChar* moduleName, const HChar* moduleCode)
{
	m_VM->ParseString(moduleName, moduleCode);
	m_CompilerBaseModules[moduleName] = GetModule(moduleName);

	return m_CompilerBaseModules[moduleName];
}

HazeCompilerModule* HazeCompiler::ParseModule(const HString& modulePath)
{
	auto pos = modulePath.find_last_of(HAZE_MODULE_PATH_CONBINE);
	HString moduleName;
	if (pos == std::string::npos)
	{
		moduleName = modulePath;
	}
	else
	{
		moduleName = modulePath.substr(pos + 1);
	}

	m_VM->ParseFile(GetModuleFilePath(moduleName, &GetCurrModule()->GetPath(), pos == std::string::npos ? nullptr : &modulePath));
	return GetModule(moduleName);
}

void HazeCompiler::FinishModule()
{
	m_CompilerModules[GetCurrModuleName()]->FinishModule();
	ClearBlockPoint();
}

HazeCompilerModule* HazeCompiler::GetModule(const HString& name)
{
	auto Iter = m_CompilerModules.find(name);
	if (Iter != m_CompilerModules.end())
	{
		return Iter->second.get();
	}

	return nullptr;
}

const HString* HazeCompiler::GetModuleName(const HazeCompilerModule* compilerModule) const
{
	for (auto& Iter : m_CompilerModules)
	{
		if (Iter.second.get() == compilerModule)
		{
			return &(Iter.first);
		}
	}

	return nullptr;
}

const HString* HazeCompiler::GetModuleTableClassName(const HString& name)
{
	auto cla = GetCurrModule()->GetClass(name);
	if (cla)
	{
		cla->GetName();
	}
	return nullptr;
}

const HString* HazeCompiler::GetModuleTableEnumName(const HString& name)
{
	auto e = HazeCompilerModule::GetEnum(GetCurrModule().get(), name);
	if (e)
	{
		e->GetName();
	}
	return nullptr;
}

void HazeCompiler::RegisterClassToSymbolTable(const HString& className)
{
	for (auto iter = m_CacheSymbols.begin(); iter != m_CacheSymbols.end(); iter++)
	{
		if (*iter == className)
		{
			return;
		}
	}

	m_CacheSymbols.push_back({ className });
	m_SymbolTable[{ &m_CacheSymbols.back() }] = nullptr;
}

void HazeCompiler::OnCreateClass(Share<HazeCompilerClass> compClass)
{
	auto iter = m_SymbolTable.find({ &compClass->GetName() });
	if (iter->second == nullptr)
	{
		iter->second = compClass;
		return;
	}
	else
	{
		COMPILER_ERR_MODULE_W("�ظ�������<%s>", compClass->GetName().c_str(), GetCurrModuleName().c_str());
		return;
	}
}

const HString* HazeCompiler::GetSymbolTableNameAddress(const HString& className)
{
	auto iter = m_SymbolTable.find({ &className });
	if (iter != m_SymbolTable.end())
	{
		return iter->first.Str;
	}
	return nullptr;
}

Share<HazeCompilerEnum> HazeCompiler::GetBaseModuleEnum(const HString& name)
{
	for (auto& it : m_CompilerBaseModules)
	{
		auto ret = it.second->GetEnum_Internal(name);
		if (ret)
		{
			return ret;
		}
	}
	
	return nullptr;
}

Share<HazeCompilerValue> HazeCompiler::GetBaseModuleGlobalVariable(const HString& name)
{
	for (auto& it : m_CompilerBaseModules)
	{
		auto ret = it.second->GetGlobalVariable_Internal(name);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

Share<HazeCompilerClass> HazeCompiler::GetBaseModuleClass(const HString& className)
{
	for (auto& it : m_CompilerBaseModules)
	{
		auto ret = it.second->GetClass(className);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

bool HazeCompiler::GetBaseModuleGlobalVariableName(const Share<HazeCompilerValue>& value, HString& outName, bool getOffset, V_Array<uint64>* offsets)
{
	for (auto& it : m_CompilerBaseModules)
	{
		if (it.second->GetGlobalVariableName_Internal(value, outName, getOffset, offsets))
		{
			return true;
		}
	}

	return false;
}

void HazeCompiler::GetRealTemplateTypes(const TemplateDefineTypes& types, V_Array<HazeDefineType>& defineTypes)
{
	for (auto& type : types.Types)
	{
		if (type.IsDefines)
		{
			HazeDefineType t;
			t.PrimaryType = HazeValueType::Function;

			HString className;
			GetTemplateClassName(className, type.Defines->Types);
		}
		else
		{
			defineTypes.push_back(*type.Type);
		}
	}
}

void HazeCompiler::InsertLineCount(int64 lineCount)
{
	if (m_VM->IsDebug() && m_InsertBaseBlock)
	{
		HAZE_LOG_INFO(H_TEXT("�� %d\n"), lineCount);
		m_InsertBaseBlock->PushIRCode(GetInstructionString(InstructionOpCode::LINE) + (H_TEXT(" ") + HAZE_TO_HAZE_STR(lineCount)) + H_TEXT("\n"));
	}
}

bool HazeCompiler::IsDebug() const
{
	return m_VM->IsDebug();
}

Unique<HazeCompilerModule>& HazeCompiler::GetCurrModule()
{
	return m_CompilerModules[GetCurrModuleName()];
}

bool HazeCompiler::CurrModuleIsStdLib()
{
	return GetCurrModule()->GetModuleLibraryType() == HazeLibraryType::Static;
}

Pair<Share<HazeCompilerFunction>, Share<HazeCompilerValue>> HazeCompiler::GetFunction(const HString& name)
{
	for (auto& iter : m_CompilerModules)
	{
		auto function = iter.second->GetFunction(name);
		if (function.first)
		{
			return function;
		}
	}

	return { nullptr, nullptr };
}

Share<HazeCompilerValue> HazeCompiler::GetNewRegister(HazeCompilerModule* compilerModule, const HazeDefineType& data)
{
	auto iter = g_GlobalRegisters.find(NEW_REGISTER);
	if (iter != g_GlobalRegisters.end())
	{
		HazeDefineType type(data);
		iter->second = CreateVariable(compilerModule, HazeDefineVariable(type, NEW_REGISTER), HazeVariableScope::Global, HazeDataDesc::RegisterNew, 0);

		return iter->second;
	}

	return nullptr;
}

Share<HazeCompilerValue> HazeCompiler::GetTempRegister(const HazeDefineType& type)
{
	auto& currModule = GetCurrModule();
	if (currModule)
	{
		auto currFunc = currModule->GetCurrFunction();
		if (currFunc)
		{
			//�ֲ���ʱ�Ĵ���
			return currFunc->CreateTempRegister(type);
		}
		else
		{
			//ȫ����ʱ�Ĵ���
		}
	}
	else
	{
		HAZE_LOG_ERR_W("���ܻ����ʱ�Ĵ���!\n");
	}
	/*for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second.use_count() == 1)
		{
			iter.second->SetDataDesc(HazeDataDesc::RegisterTemp);
			return iter.second;
		}
	}*/

	HAZE_LOG_ERR_W("���ܻ����ʱ�Ĵ���!\n");
	return nullptr;
}

HashMap<const HChar*, Share<HazeCompilerValue>> HazeCompiler::GetUseTempRegister()
{
	HashMap<const HChar*, Share<HazeCompilerValue>> registers;
	for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second.use_count() >= 2)
		{
			registers.insert({ iter.first, iter.second });
		}
	}

	return registers;
}

void HazeCompiler::ClearTempRegister(const HashMap<const HChar*, Share<HazeCompilerValue>>& useTempRegisters)
{
	for (auto & useRegi : useTempRegisters)
	{
		for (auto& regi : g_GlobalTempRegisters)
		{
			if (regi.first == useRegi.first)
			{
				regi.second = CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
					H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0);
				break;
			}
		}
	}
}

void HazeCompiler::ResetTempRegister(const HashMap<const HChar*, Share<HazeCompilerValue>>& useTempRegisters)
{
	for (auto& useRegi : useTempRegisters)
	{
		for (auto& regi : g_GlobalTempRegisters)
		{
			if (regi.first == useRegi.first)
			{
				regi.second = useRegi.second;
				break;
			}
		}
	}
}

Share<HazeCompilerValue> HazeCompiler::GetRegister(const HChar* name)
{
	auto iter = g_GlobalRegisters.find(name);
	if (iter != g_GlobalRegisters.end())
	{
		return iter->second;
	}

	return nullptr;
}

const HChar* HazeCompiler::GetRegisterName(const Share<HazeCompilerValue>& compilerRegister)
{
	for (auto& iter : g_GlobalRegisters)
	{
		if (iter.second == compilerRegister)
		{
			return iter.first;
		}
	}

	for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second == compilerRegister)
		{
			return iter.first;
		}
	}

	HAZE_LOG_ERR_W("���ҼĴ�������,δ���ҵ�!\n");
	return nullptr;
}

Share<HazeCompilerInitListValue> HazeCompiler::GetInitializeListValue()
{
	return s_InitializeListValue;
}

bool HazeCompiler::IsClass(const HString& name)
{
	auto& currModule = GetCurrModule();
	if (currModule->GetClass(name))
	{
		return true;
	}
	else
	{
		for (auto& iter : currModule->m_ImportModules)
		{
			if (iter->GetClass(name))
			{
				return true;
			}
		}

		for (auto& iter : m_CompilerBaseModules)
		{
			if (iter.second->GetClass(name))
			{
				return true;
			}
		}
	}

	return false;
}

bool HazeCompiler::IsEnum(const HString& name)
{
	auto& currModule = GetCurrModule();
	if (currModule.get()->GetEnum(currModule.get(), name))
	{
		return true;
	}

	return false;
}

bool HazeCompiler::IsTemplateClass(const HString& name)
{
	auto& currModule = GetCurrModule();
	if (currModule->IsTemplateClass(name))
	{
		return true;
	}
	else
	{
		for (auto& iter : currModule->m_ImportModules)
		{
			if (iter->IsTemplateClass(name))
			{
				return true;
			}
		}

		for (auto& iter : m_CompilerBaseModules)
		{
			if (iter.second->IsTemplateClass(name))
			{
				return true;
			}
		}
	}

	return false;
}

void HazeCompiler::MarkParseTemplate(bool begin, const HString* moduleName)
{
	static HString cacheFunctionName;
	static Share<HazeBaseBlock> cacheInsertBlock;

	if (begin)
	{
		cacheFunctionName = GetCurrModule()->m_CurrFunction;
		cacheInsertBlock = m_InsertBaseBlock;
		m_ModuleNameStack.push_back(*moduleName);
		GetCurrModule()->RestartTemplateModule(*moduleName);
	}
	else
	{
		m_ModuleNameStack.pop_back();
		GetCurrModule()->m_CurrFunction = cacheFunctionName;
		m_InsertBaseBlock = cacheInsertBlock;
		cacheFunctionName.clear();
		cacheInsertBlock = nullptr;
	}
}

Share<HazeCompilerValue> HazeCompiler::GenConstantValue(HazeValueType type, const HazeValue& var, HazeValueType* varType)
{
	static HazeDefineVariable s_DefineVariable;
	static HazeValue  s_Value;

	ConvertBaseTypeValue(type, s_Value, varType ? *varType : type, var);

	HazeValue& v = const_cast<HazeValue&>(var);
	s_DefineVariable.Type.PrimaryType = type;

	Share<HazeCompilerValue> ret = nullptr;
	switch (type)
	{
	case HazeValueType::Bool:
	{
		auto it = m_BoolConstantValues.find(s_Value.Value.Bool);
		if (it != m_BoolConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_BoolConstantValues[s_Value.Value.Bool] = ret;
	}
		break;
	case HazeValueType::Int8:
	{
		auto it = m_Int8_ConstantValues.find(s_Value.Value.Int8);
		if (it != m_Int8_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Int8_ConstantValues[s_Value.Value.Int8] = ret;
	}
		break;
	case HazeValueType::UInt8:
	{
		auto it = m_UInt8_ConstantValues.find(s_Value.Value.UInt8);
		if (it != m_UInt8_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_UInt8_ConstantValues[s_Value.Value.UInt8] = ret;
	}
		break;
	case HazeValueType::Int16:
	{
		auto it = m_Int16_ConstantValues.find(s_Value.Value.Int16);
		if (it != m_Int16_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Int16_ConstantValues[s_Value.Value.Int16] = ret;
	}
		break;
	case HazeValueType::UInt16:
	{
		auto it = m_UInt16_ConstantValues.find(s_Value.Value.UInt16);
		if (it != m_UInt16_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_UInt16_ConstantValues[s_Value.Value.UInt16] = ret;
	}
		break;
	case HazeValueType::Int32:
	{
		auto it = m_Int32_ConstantValues.find(s_Value.Value.Int32);
		if (it != m_Int32_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Int32_ConstantValues[s_Value.Value.Int32] = ret;
	}
		break;
	case HazeValueType::UInt32:
	{
		auto it = m_UInt32_ConstantValues.find(s_Value.Value.UInt32);
		if (it != m_UInt32_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_UInt32_ConstantValues[s_Value.Value.UInt32] = ret;
	}
		break;
	case HazeValueType::Int64:
	{
		auto it = m_Int64_ConstantValues.find(s_Value.Value.Int64);
		if (it != m_Int64_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Int64_ConstantValues[s_Value.Value.Int64] = ret;
	}
		break;
	case HazeValueType::UInt64:
	{
		auto it = m_UInt64_ConstantValues.find(s_Value.Value.UInt64);
		if (it != m_UInt64_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_UInt64_ConstantValues[s_Value.Value.UInt64] = ret;
	}
		break;
	case HazeValueType::Float32:
	{
		auto it = m_Float32_ConstantValues.find(s_Value.Value.Float32);
		if (it != m_Float32_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Float32_ConstantValues[s_Value.Value.Float32] = ret;
	}
		break;
	case HazeValueType::Float64:
	{
		auto it = m_Float64_ConstantValues.find(s_Value.Value.Float64);
		if (it != m_Float64_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Float64_ConstantValues[s_Value.Value.Float64] = ret;
	}
		break;
	default:
		HAZE_LOG_ERR_W("δ֧������<%s>��������!\n", GetHazeValueTypeString(type));
		break;
	}

	memcpy(((void*)&ret->GetValue()), &s_Value, sizeof(s_Value));
	return ret;
}

Share<HazeCompilerValue> HazeCompiler::GenStringVariable(HString& str)
{
	return GetCurrModule()->GetOrCreateGlobalStringVariable(str);
}

Share<HazeCompilerValue> HazeCompiler::GetGlobalVariable(const HString& name)
{
	return HazeCompilerModule::GetGlobalVariable(GetCurrModule().get(), name);
}

Share<HazeCompilerValue> HazeCompiler::GetLocalVariable(const HString& name)
{
	return  GetCurrModule()->GetCurrFunction()->GetLocalVariable(name);
}

Share<HazeCompilerValue> HazeCompiler::GetEnumVariable(const HString& enumName, const HString& name)
{
	return  GetCurrModule()->GetEnum(GetCurrModule().get(), enumName)->GetEnumValue(name);
}

Share<HazeCompilerValue> HazeCompiler::GetConstantValueInt(int v)
{
	HazeValue Value;
	Value.Value.Int32 = v;

	return GenConstantValue(HazeValueType::Int32, Value);
}

Share<HazeCompilerValue> HazeCompiler::GetConstantValueUint64(uint64 v)
{
	HazeValue Value;
	Value.Value.UInt64 = v;

	return GenConstantValue(HazeValueType::UInt64, Value);
}

Share<HazeCompilerValue> HazeCompiler::GenConstantValueBool(bool isTrue)
{
	HazeValue Value;
	Value.Value.Bool = isTrue;

	return GenConstantValue(HazeValueType::Bool, Value);
}

Share<HazeCompilerValue> HazeCompiler::GetNullPtr(const HazeDefineType& type)
{
	static HashMap<HazeDefineType, Share<HazeCompilerValue>, HazeDefineTypeHashFunction> s_NullPtrs;

	auto Iter = s_NullPtrs.find(type);
	if (Iter != s_NullPtrs.end())
	{
		return Iter->second;
	}

	s_NullPtrs[type] = CreateVariable(nullptr, HazeDefineVariable(type, H_TEXT("")), HazeVariableScope::Global, HazeDataDesc::NullPtr, 0);
	return s_NullPtrs[type];
}

bool HazeCompiler::IsConstantValueBoolTrue(Share<HazeCompilerValue> v)
{
	return v == GenConstantValueBool(true);
}

bool HazeCompiler::IsConstantValueBoolFalse(Share<HazeCompilerValue> v)
{
	return v == GenConstantValueBool(false);
}

void HazeCompiler::SetInsertBlock(Share<HazeBaseBlock> block)
{
	m_InsertBaseBlock = block;
}

void HazeCompiler::ClearBlockPoint()
{
	m_InsertBaseBlock = nullptr;
}

void HazeCompiler::AddImportModuleToCurrModule(HazeCompilerModule* compilerModule)
{
	for (auto& iter : GetCurrModule()->m_ImportModules)
	{
		if (iter == compilerModule)
		{
			return;
		}
	}

	GetCurrModule()->m_ImportModules.push_back(compilerModule);
}

Share<HazeCompilerValue> HazeCompiler::CreateLea(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value)
{
	/*if (value->IsArrayElement())
	{
		return GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, CreatePointerToArrayElement(value), InstructionOpCode::MOV);
	}
	else*/ if (value->IsArray())
	{
		return CreateMov(allocaValue, value);
	}
	else
	{
		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::LEA);
		return allocaValue;
	}
}

Share<HazeCompilerValue> HazeCompiler::CreateMov(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value, bool storeValue)
{
	if (allocaValue->IsRefrence() && !value->IsRefrence())
	{
		if (value->IsConstant())
		{
			CreateMovToPV(allocaValue, value);
		}
		else
		{
			CreateLea(allocaValue, value);
		}
	}
	else
	{
		if (CanCVT(allocaValue->GetValueType().PrimaryType, value->GetValueType().PrimaryType))
		{
			value = CreateCVT(allocaValue, value);
		}

		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::MOV);
	}

	return allocaValue;
}

Share<HazeCompilerValue> HazeCompiler::CreateMovToPV(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value)
{
	GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::MOVTOPV);
	return allocaValue;
}

Share<HazeCompilerValue> HazeCompiler::CreateMovPV(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value)
{
	//if (value->IsRefrence())
	{
		HazeDefineType& allocaValueType = const_cast<HazeDefineType&>(allocaValue->GetValueType());
		auto& ValueValueType = value->GetValueType();

		
		{
			allocaValueType.PrimaryType = HazeValueType::UInt64;//ValueValueType.SecondaryType;
			allocaValueType.CustomName = nullptr;//ValueValueType.CustomName;

			allocaValueType.SecondaryType = HazeValueType::None;
		}

		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::MOVPV);
		return allocaValue;
	}
	/*else
	{
		COMPILER_ERR_MODULE_W("����<%s>��������", GetInstructionString(InstructionOpCode::MOVPV), GetCurrModuleName().c_str());
	}*/

	return allocaValue;
}


Share<HazeCompilerValue> HazeCompiler::CreateVariableBySection(HazeSectionSignal section, Unique<HazeCompilerModule>& mod, Share<HazeCompilerFunction> func,
	const HazeDefineVariable& var, int line, Share<HazeCompilerValue> refValue, V_Array<Share<HazeCompilerValue>> arraySize, V_Array<HazeDefineType>* params)
{
	switch (section)
	{
	case HazeSectionSignal::Global:
		return CreateGlobalVariable(mod, var, refValue, arraySize, params);
	case HazeSectionSignal::Local:
		return CreateLocalVariable(func, var, line, refValue, arraySize, params);
	case HazeSectionSignal::Static:
		break;
	case HazeSectionSignal::Class:
		return CreateClassVariable(mod, var, refValue, arraySize, params);
	case HazeSectionSignal::Enum:
		break;
	default:
		break;
	}

	return nullptr;
}

Share<HazeCompilerValue> HazeCompiler::CreateLocalVariable(Share<HazeCompilerFunction> function, const HazeDefineVariable& variable, int line,
	Share<HazeCompilerValue> refValue, V_Array<Share<HazeCompilerValue>> arraySize, V_Array<HazeDefineType>* params)
{
	return function->CreateLocalVariable(variable, line, refValue, arraySize, params);
}

Share<HazeCompilerValue> HazeCompiler::CreateGlobalVariable(Unique<HazeCompilerModule>& compilerModule, const HazeDefineVariable& var,
	Share<HazeCompilerValue> refValue, V_Array<Share<HazeCompilerValue>> arraySize, V_Array<HazeDefineType>* params)
{
	return compilerModule->CreateGlobalVariable(var, refValue, arraySize, params);
}

Share<HazeCompilerValue> HazeCompiler::CreateClassVariable(Unique<HazeCompilerModule>& compilerModule, const HazeDefineVariable& var,
	Share<HazeCompilerValue> refValue, V_Array<Share<HazeCompilerValue>> arraySize, V_Array<HazeDefineType>* params)
{
	return CreateVariable(compilerModule.get(), var, HazeVariableScope::None, HazeDataDesc::Class, 0, refValue, arraySize, params);
}

Share<HazeCompilerValue> HazeCompiler::CreateRet(Share<HazeCompilerValue> value)
{
	if (value->IsArrayElement())
	{
		value = GetArrayElementToValue(GetCurrModule().get(), value);
	}
	GetCurrModule()->GenIRCode_Ret(value);
	return value;
}

Share<HazeCompilerValue> HazeCompiler::CreateAdd(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	ReplaceConstantValueByStrongerType(oper1, oper2);
	GetCurrModule()->CreateAdd(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateSub(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	ReplaceConstantValueByStrongerType(oper1, oper2);
	GetCurrModule()->CreateSub(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateMul(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	ReplaceConstantValueByStrongerType(oper1, oper2);
	GetCurrModule()->CreateMul(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateDiv(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	ReplaceConstantValueByStrongerType(oper1, oper2);
	GetCurrModule()->CreateDiv(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateMod(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	GetCurrModule()->CreateMod(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateBitAnd(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	GetCurrModule()->CreateBitAnd(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateBitOr(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	GetCurrModule()->CreateBitOr(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateBitNeg(Share<HazeCompilerValue> value)
{
	GetCurrModule()->CreateBitNeg(value);
	return value;
}

Share<HazeCompilerValue> HazeCompiler::CreateBitXor(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	GetCurrModule()->CreateBitXor(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateShl(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	GetCurrModule()->CreateShl(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateShr(Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1, Share<HazeCompilerValue> oper2)
{
	GetCurrModule()->CreateShr(assignTo, oper1, oper2);
	return assignTo;
}

Share<HazeCompilerValue> HazeCompiler::CreateNot(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right)
{
	return GetCurrModule()->CreateNot(left, right);
}

Share<HazeCompilerValue> HazeCompiler::CreateFunctionCall(Share<HazeCompilerFunction> function, V_Array<Share<HazeCompilerValue>>& param, Share<HazeCompilerValue> thisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(function, param, thisPointerTo);
}

Share<HazeCompilerValue> HazeCompiler::CreateFunctionCall(Share<HazeCompilerValue> pointerFunction, V_Array<Share<HazeCompilerValue>>& param, Share<HazeCompilerValue> thisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(pointerFunction, param, thisPointerTo);
}

Share<HazeCompilerValue> HazeCompiler::CreateAdvanceTypeFunctionCall(HazeValueType advanceType, const HString& functionName, V_Array<Share<HazeCompilerValue>>& param, Share<HazeCompilerValue> thisPointerTo)
{
	auto iter = m_AdvanceClassInfo.find(advanceType);
	if (iter != m_AdvanceClassInfo.end())
	{
		auto func = iter->second.Functions.find(functionName);
		if (func != iter->second.Functions.end())
		{
			return GetCurrModule()->CreateAdvanceTypeFunctionCall(func->second, param, thisPointerTo);
		}
		else
		{
			COMPILER_ERR_MODULE_W("����<%s>û���ҵ�<%s>����", GetHazeValueTypeString(advanceType), functionName.c_str(),
				GetCurrModuleName().c_str());
		}
	}
	else
	{
		COMPILER_ERR_MODULE_W("����<%s>û���ҵ�������Ϣ", GetHazeValueTypeString(advanceType), GetCurrModuleName().c_str());
	}

	return HazeCompiler::GetRegister(RET_REGISTER);
}

Share<HazeCompilerValue> HazeCompiler::CreateArrayInit(Share<HazeCompilerValue> arrValue, Share<HazeCompilerValue> initList)
{
	return GetCurrModule()->CreateArrayInit(arrValue, initList);
}

Share<HazeCompilerValue> HazeCompiler::CreateArrayElement(Share<HazeCompilerValue> value, V_Array<uint32> index)
{
	V_Array<Share<HazeCompilerValue>> indces;
	for (size_t i = 0; i < index.size(); i++)
	{
		HazeValue IndexValue;
		IndexValue.Value.UInt64 = index[i];

		indces.push_back(GenConstantValue(HazeValueType::UInt64, IndexValue));
	}

	return CreateArrayElement(value, indces);
}

Share<HazeCompilerValue> HazeCompiler::CreateArrayElement(Share<HazeCompilerValue> value, V_Array<Share<HazeCompilerValue>> indices)
{
	V_Array<HazeCompilerValue*> values;
	for (auto& iter : indices)
	{
		auto  index = iter;
		/*if (index->IsConstant() && index->GetValueType().PrimaryType != HazeValueType::UInt64)
		{
			switch (iter->GetValueType().PrimaryType)
			{
			case HazeValueType::Int32:
				index = GetConstantValueUint64(index->GetValue().Value.Int32);
				break;
			default:
				HAZE_LOG_ERR_W("��������������������!");
				break;
			}
		}*/

		values.push_back(index.get());
	}

	if (value->IsArray())
	{
		auto arrayValue = DynamicCast<HazeCompilerArrayValue>(value);

		HazeDefineType type(arrayValue->GetValueType().SecondaryType);
		type.CustomName = arrayValue->GetValueType().CustomName;
		/*if (IsArrayPointerType(arrayValue->GetValueType().PrimaryType))
		{
			if (type.HasCustomName())
			{
				type.PrimaryType = HazeValueType::PointerClass;
			}
			else if (IsHazeBaseTypeAndVoid(type.PrimaryType))
			{
				type.SecondaryType = type.PrimaryType;
				type.PrimaryType = HazeValueType::PointerBase;
			}
			else
			{
				COMPILER_ERR_MODULE_W("�����������", GetCurrModuleName().c_str());
			}
		}*/

		return MakeShare<HazeCompilerArrayElementValue>(GetCurrModule().get(), type, value->GetVariableScope(),
			HazeDataDesc::ArrayElement, 0, value.get(), values);
	}

	return nullptr;
}

Share<HazeCompilerValue> HazeCompiler::CreatePointerToValue(Share<HazeCompilerValue> value)
{
	if (IsHazeBaseType(value->GetValueType().PrimaryType))
	{
		auto pointer = GetTempRegister(value->GetValueType());
		auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
		type.Pointer();
		return CreateLea(pointer, value);
	}
	else
	{
		COMPILER_ERR_MODULE_W("���ܶԷǻ�����ȡ��ַ", GetCurrModuleName().c_str());
	}
}

//Share<HazeCompilerValue> HazeCompiler::CreatePointerToArray(Share<HazeCompilerValue> arrValue, Share<HazeCompilerValue> index)
//{
//	auto pointer = GetTempRegister();
//
//	auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
//	type.PointerTo(arrValue->GetValueType());
//
//	auto ret = CreateLea(pointer, arrValue);
//	if (index)
//	{
//		CreateAdd(ret, index);
//	}
//
//	return ret;
//}

//Share<HazeCompilerValue> HazeCompiler::CreatePointerToArrayElement(Share<HazeCompilerValue> elementValue)
//{
//	auto arrayElementValue = DynamicCast<HazeCompilerArrayElementValue>(elementValue);
//	auto arrayValue = DynamicCast<HazeCompilerArrayValue>(arrayElementValue->GetArray()->GetShared());
//
//	if (arrayValue)
//	{
//		auto tempRegister = GetTempRegister();
//		Share<HazeCompilerValue> arrayPointer = CreatePointerToArray(arrayValue);
//
//		for (uint64 i = 0; i < arrayElementValue->GetIndex().size(); i++)
//		{
//			uint64 size = i == arrayElementValue->GetIndex().size() - 1 ? arrayElementValue->GetIndex()[i]->GetValue().Value.UnsignedLong
//				: arrayValue->GetSizeByLevel((uint64)i);
//			Share<HazeCompilerValue> sizeValue = nullptr;
//
//			if (arrayElementValue->GetIndex()[i]->IsConstant())
//			{
//				sizeValue = GetConstantValueUint64(size);
//			}
//			else
//			{
//				sizeValue = i == arrayElementValue->GetIndex().size() - 1 ? arrayElementValue->GetIndex()[i]->GetShared() : GetConstantValueUint64(size);
//			}
//
//			CreateMov(tempRegister, sizeValue);
//			if (i != arrayElementValue->GetIndex().size() - 1)
//			{
//				CreateMul(tempRegister, arrayElementValue->GetIndex()[i]->GetShared());
//			}
//			CreateAdd(arrayPointer, tempRegister);
//		}
//
//		return arrayPointer;
//	}
//
//	return nullptr;
//}

//Share<HazeCompilerValue> HazeCompiler::CreatePointerToPointerArray(Share<HazeCompilerValue> pointerArray,
//	Share<HazeCompilerValue> index)
//{
//	auto pointer = GetTempRegister();
//
//	auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
//	type.PrimaryType = HazeValueType::PointerBase;
//
//	auto ret = CreateMov(pointer, pointerArray);
//	if (index)
//	{
//		CreateAdd(ret, index);
//	}
//
//	return ret;
//}

Share<HazeCompilerValue> HazeCompiler::CreatePointerToFunction(Share<HazeCompilerFunction> function, Share<HazeCompilerValue> pointer)
{
	auto tempPointer = GetTempRegister(HazeDefineType(HazeValueType::Function, &function->GetName()));
	tempPointer->SetDataDesc(HazeDataDesc::FunctionAddress);
	
	return CreateLea(pointer, tempPointer);
}

Share<HazeCompilerValue> HazeCompiler::CreateNew(Share<HazeCompilerFunction> function, const HazeDefineType& data,
	V_Array<Share<HazeCompilerValue>>* countValue)
{
	return function->CreateNew(data, countValue);
}

Share<HazeCompilerValue> HazeCompiler::CreateCast(const HazeDefineType& type, Share<HazeCompilerValue> value)
{
	auto reg = GetTempRegister(type);
	GetCurrModule()->GenIRCode_BinaryOperater(reg, reg, value, InstructionOpCode::CVT);

	return reg;
}

Share<HazeCompilerValue> HazeCompiler::CreateCVT(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right)
{
	auto tempReg = GetTempRegister(left->GetValueType());
	GetCurrModule()->GenIRCode_BinaryOperater(tempReg, tempReg, right, InstructionOpCode::CVT);

	return tempReg;
}

Share<HazeCompilerValue> HazeCompiler::CreateNeg(Share<HazeCompilerValue> value)
{
	return GetCurrModule()->CreateNeg(value);
}

Share<HazeCompilerValue> HazeCompiler::CreateInc(Share<HazeCompilerValue> value, bool isPreInc)
{
	return GetCurrModule()->CreateInc(value, isPreInc);
}

Share<HazeCompilerValue> HazeCompiler::CreateDec(Share<HazeCompilerValue> value, bool isPreDec)
{
	return GetCurrModule()->CreateDec(value, isPreDec);
}

void HazeCompiler::CreateJmpToBlock(Share<HazeBaseBlock> block)
{
	GetCurrModule()->GenIRCode_JmpTo(block);
}

Share<HazeCompilerValue> HazeCompiler::CreateIntCmp(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right)
{
	if (left->IsArrayElement())
	{
		left = GetArrayElementToValue(GetCurrModule().get(), left);
	}

	if (right->IsArrayElement())
	{
		right = GetArrayElementToValue(GetCurrModule().get(), right);
	}

	GetCurrModule()->GenIRCode_BinaryOperater(nullptr, left, right, InstructionOpCode::CMP);
	return GetRegister(CMP_REGISTER);
}

Share<HazeCompilerValue> HazeCompiler::CreateBoolCmp(Share<HazeCompilerValue> value)
{
	if (IsConstantValueBoolTrue(value))
	{
		auto v = GenConstantValueBool(true);
		auto temp = CreateMov(GetTempRegister(v->GetValueType()), v);
		CreateIntCmp(CreateBitXor(temp, temp,
			GenConstantValueBool(false)), GenConstantValueBool(true));
		return GenConstantValueBool(true);
	}
	else if (IsConstantValueBoolFalse(value))
	{
		auto v = GenConstantValueBool(true);
		auto temp = CreateMov(GetTempRegister(v->GetValueType()), v);
		CreateIntCmp(CreateBitXor(temp, temp,
			GenConstantValueBool(false)), GenConstantValueBool(false));
		return GenConstantValueBool(false);
	}
	else
	{
		return CreateIntCmp(value, GenConstantValueBool(true));
	}
}

void HazeCompiler::ReplaceConstantValueByStrongerType(Share<HazeCompilerValue>& left, Share<HazeCompilerValue>& right)
{
	if ((IsArrayType(left->GetValueType().PrimaryType) && IsNumberType(right->GetValueType().PrimaryType)))
	{
		return;
	}

	if (left->GetValueType().PrimaryType != right->GetValueType().PrimaryType)
	{
		auto strongerType = GetStrongerType(left->GetValueType().PrimaryType, right->GetValueType().PrimaryType);
		if (left->GetValueType().PrimaryType == strongerType)
		{
			if (right->IsConstant())
			{
				HazeValue v;
				ConvertBaseTypeValue(strongerType, v, right->GetValueType().PrimaryType, right->GetValue());
				right = GenConstantValue(strongerType, v);
			}
			else
			{
				auto tempRegister = GetTempRegister(right->GetValueType());
				right = CreateMov(tempRegister, right, false);
			}
		}
		else
		{
			if (left->IsConstant())
			{
				HazeValue v;
				ConvertBaseTypeValue(strongerType, v, left->GetValueType().PrimaryType, left->GetValue());
				left = GenConstantValue(strongerType, v);
			}
			else
			{
				auto tempRegister = GetTempRegister(right->GetValueType());
				left = CreateMov(tempRegister, left, false);
			}
		}
	}
	
}

void HazeCompiler::CreateCompareJmp(HazeCmpType cmpType, Share<HazeBaseBlock> ifJmpBlock, Share<HazeBaseBlock> elseJmpBlock)
{
	GetCurrModule()->GenIRCode_Cmp(cmpType, ifJmpBlock, elseJmpBlock);
}