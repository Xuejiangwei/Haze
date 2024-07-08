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

static HashMap<const HChar*, Share<HazeCompilerValue>> g_GlobalRegisters = {
	{ RET_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("Ret_Register")), H_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterRet, 0) },
	{ NEW_REGISTER, nullptr },
	{ CMP_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("Cmp_Register")), H_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterCmp, 0) },
};

static HashMap<const HChar*, Share<HazeCompilerValue>> g_GlobalTempRegisters = {
	{ TEMP_REGISTER_0, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_1, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_2, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_3, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_4, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_5, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_6, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_7, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_8, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_9, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")), H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
};

static Share<HazeCompilerInitListValue> s_InitializeListValue =
MakeShare<HazeCompilerInitListValue>(nullptr, HazeDefineType(HazeValueType::Void, H_TEXT("Temp_Register")), HazeVariableScope::Temp, HazeDataDesc::Initlist, 0);

HazeCompiler::HazeCompiler(HazeVM* vm) : m_VM(vm)
{
}

HazeCompiler::~HazeCompiler()
{
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
	for (auto& m : m_CompilerModules)
	{
		m.second->FinishModule();
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

bool HazeCompiler::GetBaseModuleGlobalVariableName(const Share<HazeCompilerValue>& value, HString& outName)
{
	for (auto& it : m_CompilerBaseModules)
	{
		if (it.second->GetGlobalVariableName_Internal(value, outName))
		{
			return true;
		}
	}

	return false;
}

void HazeCompiler::InsertLineCount(int64 lineCount)
{
	if (m_VM->IsDebug() && m_InsertBaseBlock)
	{
		HAZE_LOG_INFO("HazeCompiler line %d\n", lineCount);
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
	return GetCurrModule()->GetModuleLibraryType() == HazeLibraryType::Standard;
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
		type.PointerTo(data);
		iter->second = CreateVariable(compilerModule, HazeDefineVariable(type, H_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterNew, 0);

		return iter->second;
	}

	return nullptr;
}

Share<HazeCompilerValue> HazeCompiler::GetTempRegister()
{
	for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second.use_count() == 1)
		{
			return iter.second;
		}
	}

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
				regi.second = CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, H_TEXT("")),
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

const HChar* HazeCompiler::GetClassName(const HString& name)
{
	for (auto& iter : m_CompilerModules)
	{
		auto compilerClass = iter.second->GetClass(name);
		if (compilerClass)
		{
			return compilerClass->GetName().c_str();
		}
	}

	return nullptr;
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

Share<HazeCompilerValue> HazeCompiler::GenConstantValue(HazeValueType type, const HazeValue& var)
{
	static HazeDefineVariable s_DefineVariable;

	HazeValue& v = const_cast<HazeValue&>(var);
	s_DefineVariable.Type.PrimaryType = type;

	Share<HazeCompilerValue> ret = nullptr;
	switch (type)
	{
	case HazeValueType::Bool:
	{
		auto it = m_BoolConstantValues.find(var.Value.Bool);
		if (it != m_BoolConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_BoolConstantValues[var.Value.Bool] = ret;
		return ret;
	}
	case HazeValueType::Byte:
	{
		auto it = m_ByteConstantValues.find(var.Value.Byte);
		if (it != m_ByteConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_ByteConstantValues[var.Value.Byte] = ret;
		return ret;
	}
	case HazeValueType::UnsignedByte:
	{
		auto it = m_UnsignedByteConstantValues.find(var.Value.UnsignedByte);
		if (it != m_UnsignedByteConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_UnsignedByteConstantValues[var.Value.UnsignedByte] = ret;
		return ret;
	}
	case HazeValueType::Char:
	{
		auto it = m_CharConstantValues.find(var.Value.Char);
		if (it != m_CharConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_CharConstantValues[var.Value.Char] = ret;
		return ret;
	}
	case HazeValueType::Short:
	{
		auto it = m_ShortConstantValues.find(var.Value.Short);
		if (it != m_ShortConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_ShortConstantValues[var.Value.Short] = ret;
		return ret;
	}
	case HazeValueType::UnsignedShort:
	{
		auto it = m_UnsignedShortConstantValues.find(var.Value.UnsignedShort);
		if (it != m_UnsignedShortConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_UnsignedShortConstantValues[var.Value.UnsignedShort] = ret;
		return m_UnsignedShortConstantValues[var.Value.UnsignedShort];
	}
	case HazeValueType::Int:
	{
		auto it = m_IntConstantValues.find(var.Value.Int);
		if (it != m_IntConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_IntConstantValues[var.Value.Int] = ret;
		return m_IntConstantValues[var.Value.Int];
	}
	case HazeValueType::Long:
	{
		auto it = m_LongConstantValues.find(var.Value.Long);
		if (it != m_LongConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_LongConstantValues[var.Value.Long] = ret;
		return ret;
	}
	case HazeValueType::UnsignedInt:
	{
		auto it = m_UnsignedIntConstantValues.find(var.Value.UnsignedInt);
		if (it != m_UnsignedIntConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_UnsignedIntConstantValues[var.Value.UnsignedInt] = ret;
		return ret;
	}
	case HazeValueType::UnsignedLong:
	{
		auto it = m_UnsignedLongConstantValues.find(var.Value.UnsignedLong);
		if (it != m_UnsignedLongConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_UnsignedLongConstantValues[var.Value.UnsignedLong] = ret;
		return ret;
	}
	case HazeValueType::Float:
	{
		auto it = m_FloatConstantValues.find(var.Value.Float);
		if (it != m_FloatConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_FloatConstantValues[var.Value.Float] = ret;
		return ret;
	}
	case HazeValueType::Double:
	{
		auto it = m_DobuleConstantValues.find(var.Value.Double);
		if (it != m_DobuleConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		ret->StoreValue(v);
		m_DobuleConstantValues[var.Value.Double] = ret;
		return ret;
	}
	default:
		HAZE_LOG_ERR_W("δ֧������<%s>��������!\n", GetHazeValueTypeString(type));
		break;
	}

	return nullptr;
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
	Value.Value.Int = v;

	return GenConstantValue(HazeValueType::Int, Value);
}

Share<HazeCompilerValue> HazeCompiler::GetConstantValueUint64(uint64 v)
{
	HazeValue Value;
	Value.Value.UnsignedLong = v;

	return GenConstantValue(HazeValueType::UnsignedLong, Value);
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
	if (allocaValue->IsRef() && value->IsArrayElement())
	{
		return GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, CreatePointerToArrayElement(value), InstructionOpCode::MOV);
	}
	else if (value->IsArray())
	{
		return CreateMov(allocaValue, value);
	}
	else
	{
		return GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, InstructionOpCode::LEA);
	}
}

Share<HazeCompilerValue> HazeCompiler::CreateMov(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value, bool storeValue)
{
	if (storeValue)
	{
		allocaValue->StoreValueType(value);
	}

	if ((allocaValue->IsPointer() && !value->IsPointer() && !value->IsArray()) || (allocaValue->IsPointerPointer() && value->IsPointer() && !value->IsPointerPointer()))
	{
		CreateLea(allocaValue, value);
	}
	else if (allocaValue->IsRef())
	{
		CreateMovToPV(allocaValue, value->IsRef() ? CreateMovPV(GetTempRegister(), value) : value);
	}
	else if (value->IsArrayElement() || allocaValue->IsArrayElement())
	{
		if (allocaValue->IsArrayElement() && !value->IsArrayElement())
		{
			auto arrayPointer = CreatePointerToArrayElement(allocaValue);
			CreateMovToPV(arrayPointer, value);
		}
		else if (!allocaValue->IsArrayElement() && value->IsArrayElement())
		{
			GetArrayElementToValue(GetCurrModule().get(), value, allocaValue);
		}
		else
		{
			auto tempValue = GetArrayElementToValue(GetCurrModule().get(), value);
			auto arrayPointer = CreatePointerToArrayElement(allocaValue);
			CreateMovToPV(arrayPointer, tempValue);
		}
	}
	else
	{
		if (CanCVT(allocaValue->GetValueType().PrimaryType, value->GetValueType().PrimaryType))
		{
			value = CreateCVT(allocaValue, value);
		}

		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, InstructionOpCode::MOV);
	}

	return allocaValue;
}

Share<HazeCompilerValue> HazeCompiler::CreateMovToPV(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value)
{
	if (allocaValue->IsRef() || allocaValue->IsPointer())
	{
		return GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, InstructionOpCode::MOVTOPV);
	}

	return allocaValue;
}

Share<HazeCompilerValue> HazeCompiler::CreateMovPV(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value)
{
	if (value->IsPointer() || value->IsRef())
	{
		allocaValue->StoreValueType(value);

		HazeDefineType& allocaValueType = const_cast<HazeDefineType&>(allocaValue->GetValueType());
		auto& ValueValueType = value->GetValueType();

		if (IsPointerPointer(value->GetValueType().PrimaryType))
		{
			if (ValueValueType.HasCustomName())
			{
				allocaValueType.PrimaryType = HazeValueType::PointerClass;
				allocaValueType.CustomName = ValueValueType.CustomName;
				allocaValueType.SecondaryType = HazeValueType::Void;
			}
			else
			{
				allocaValueType.PrimaryType = HazeValueType::PointerBase;
				allocaValueType.CustomName = ValueValueType.CustomName;
				allocaValueType.SecondaryType = ValueValueType.SecondaryType;
			}
		}
		else
		{
			allocaValueType.PrimaryType = ValueValueType.SecondaryType;
			allocaValueType.CustomName = ValueValueType.CustomName;

			allocaValueType.SecondaryType = HazeValueType::Void;
		}

		return GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, InstructionOpCode::MOVPV);
	}
	else if (value->IsArray())
	{
		return CreateArrayElement(value, { GetConstantValueInt(0) });
	}

	return allocaValue;
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

Share<HazeCompilerValue> HazeCompiler::CreateAdd(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	ReplaceConstantValueByStrongerType(left, right);
	return GetCurrModule()->CreateAdd(left, right, isAssign);
}

Share<HazeCompilerValue> HazeCompiler::CreateSub(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	ReplaceConstantValueByStrongerType(left, right);

	Share<HazeCompilerValue> value = nullptr;
	if (left->IsPointer() && right->IsPointer())
	{
		auto register1 = GetTempRegister();
		auto& valueType1 = const_cast<HazeDefineType&>(register1->GetValueType());
		valueType1.PrimaryType = HazeValueType::Long;
		valueType1.SecondaryType = HazeValueType::Void;
		valueType1.CustomName.clear();

		auto register2 = GetTempRegister();
		auto& valueType2 = const_cast<HazeDefineType&>(register2->GetValueType());
		valueType2.PrimaryType = HazeValueType::Long;
		valueType2.SecondaryType = HazeValueType::Void;
		valueType2.CustomName.clear();

		GetCurrModule()->GenIRCode_BinaryOperater(register1, left, InstructionOpCode::MOV);
		GetCurrModule()->GenIRCode_BinaryOperater(register2, right, InstructionOpCode::MOV);
		value = CreateSub(register1, register2, isAssign);
		
		if (left->GetValueType() == right->GetValueType())
		{
			auto& type = left->GetValueType();
			auto size = type.HasCustomName() ? GetCurrModule()->GetClassSize(type.CustomName) : 
				IsPointerPointer(type.PrimaryType) ? GetSizeByHazeType(type.PrimaryType) : GetSizeByHazeType(type.SecondaryType);
			if (size > 1)
			{
				if ((size & (size - 1)) == 0)
				{
					value = CreateShr(value, GetConstantValueInt(Log2(size)));
				}
				else
				{
					HAZE_LOG_ERR_W("����<SUB>����, ָ��ָ������͵��ֽڴ�СӦΪ2���ݴη�!\n");
				}
			}
		}
		else
		{
			HAZE_LOG_ERR_W("����<SUB>����, ����ָ�������Ӧ����ͬ!\n");
		}
	}
	else
	{
		value = GetCurrModule()->CreateSub(left, right, isAssign);
	}

	return value;
}

Share<HazeCompilerValue> HazeCompiler::CreateMul(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	ReplaceConstantValueByStrongerType(left, right);
	return GetCurrModule()->CreateMul(left, right, isAssign);
}

Share<HazeCompilerValue> HazeCompiler::CreateDiv(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	ReplaceConstantValueByStrongerType(left, right);
	return GetCurrModule()->CreateDiv(left, right, isAssign);
}

Share<HazeCompilerValue> HazeCompiler::CreateMod(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateMod(left, right, isAssign);
}

Share<HazeCompilerValue> HazeCompiler::CreateBitAnd(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateBitAnd(left, right, isAssign);
}

Share<HazeCompilerValue> HazeCompiler::CreateBitOr(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateBitOr(left, right, isAssign);
}

Share<HazeCompilerValue> HazeCompiler::CreateBitNeg(Share<HazeCompilerValue> value)
{
	return GetCurrModule()->CreateBitNeg(value);
}

Share<HazeCompilerValue> HazeCompiler::CreateBitXor(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateBitXor(left, right, isAssign);
}

Share<HazeCompilerValue> HazeCompiler::CreateShl(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateShl(left, right, isAssign);
}

Share<HazeCompilerValue> HazeCompiler::CreateShr(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateShr(left, right, isAssign);
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
		IndexValue.Value.Int = index[i];

		indces.push_back(GenConstantValue(HazeValueType::UnsignedLong, IndexValue));
	}

	return CreateArrayElement(value, indces);
}

Share<HazeCompilerValue> HazeCompiler::CreateArrayElement(Share<HazeCompilerValue> value, V_Array<Share<HazeCompilerValue>> indices)
{
	V_Array<HazeCompilerValue*> values;
	for (auto& iter : indices)
	{
		auto  index = iter;
		if (index->IsConstant() && !IsUnsignedLongType(index->GetValueType().PrimaryType))
		{
			switch (iter->GetValueType().PrimaryType)
			{
			case HazeValueType::Int:
				index = GetConstantValueUint64(index->GetValue().Value.Int);
				break;
			default:
				HAZE_LOG_ERR_W("��������������������!");
				break;
			}
		}

		values.push_back(index.get());
	}

	if (value->IsArray())
	{
		auto arrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(value);

		HazeDefineType type(arrayValue->GetValueType().SecondaryType, arrayValue->GetValueType().CustomName);
		if (IsArrayPointerType(arrayValue->GetValueType().PrimaryType))
		{
			if (type.HasCustomName())
			{
				type.PrimaryType = HazeValueType::PointerClass;
			}
			else if (IsHazeDefaultTypeAndVoid(type.PrimaryType))
			{
				type.SecondaryType = type.PrimaryType;
				type.PrimaryType = HazeValueType::PointerBase;
			}
			else
			{
				COMPILER_ERR_MODULE_W("�����������", GetCurrModuleName().c_str());
			}
		}

		return MakeShare<HazeCompilerArrayElementValue>(GetCurrModule().get(), type, value->GetVariableScope(),
			HazeDataDesc::ArrayElement, 0, value.get(), values);
	}
	else if (value->IsPointer())
	{
		auto pointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(value);
		if (pointerValue)
		{
			return MakeShare<HazeCompilerArrayElementValue>(GetCurrModule().get(), HazeDefineType(pointerValue->GetValueType().SecondaryType,
				pointerValue->GetValueType().CustomName), value->GetVariableScope(), HazeDataDesc::ArrayElement, 0, value.get(), values);
		}
	}

	return nullptr;
}

Share<HazeCompilerValue> HazeCompiler::CreateGetArrayLength(Share<HazeCompilerValue> value)
{
	auto sizeValue = GetTempRegister();
	auto& type = const_cast<HazeDefineType&>(sizeValue->GetValueType());
	type = HazeDefineType(HazeValueType::UnsignedLong);
	GetCurrModule()->GenIRCode_BinaryOperater(sizeValue, value, InstructionOpCode::ARRAY_LENGTH);

	return sizeValue;
}

Share<HazeCompilerValue> HazeCompiler::CreatePointerToValue(Share<HazeCompilerValue> value)
{
	if (value->IsArrayElement())
	{
		return CreatePointerToArrayElement(value);
	}
	else
	{
		auto pointer = GetTempRegister();

		auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
		type.PointerTo(value->GetValueType());
		return CreateLea(pointer, value);
	}
}

Share<HazeCompilerValue> HazeCompiler::CreatePointerToArray(Share<HazeCompilerValue> arrValue, Share<HazeCompilerValue> index)
{
	auto pointer = GetTempRegister();

	auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
	type.PointerTo(arrValue->GetValueType());

	auto ret = CreateLea(pointer, arrValue);
	if (index)
	{
		CreateAdd(ret, index);
	}

	return ret;
}

Share<HazeCompilerValue> HazeCompiler::CreatePointerToArrayElement(Share<HazeCompilerValue> elementValue)
{
	auto arrayElementValue = std::dynamic_pointer_cast<HazeCompilerArrayElementValue>(elementValue);
	auto arrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(arrayElementValue->GetArray()->GetShared());

	if (arrayValue)
	{
		auto tempRegister = GetTempRegister();
		Share<HazeCompilerValue> arrayPointer = CreatePointerToArray(arrayValue);

		for (uint64 i = 0; i < arrayElementValue->GetIndex().size(); i++)
		{
			uint64 size = i == arrayElementValue->GetIndex().size() - 1 ? arrayElementValue->GetIndex()[i]->GetValue().Value.UnsignedLong
				: arrayValue->GetSizeByLevel((uint64)i);
			Share<HazeCompilerValue> sizeValue = nullptr;

			if (arrayElementValue->GetIndex()[i]->IsConstant())
			{
				sizeValue = GetConstantValueUint64(size);
			}
			else
			{
				sizeValue = i == arrayElementValue->GetIndex().size() - 1 ? arrayElementValue->GetIndex()[i]->GetShared() : GetConstantValueUint64(size);
			}

			CreateMov(tempRegister, sizeValue);
			if (i != arrayElementValue->GetIndex().size() - 1)
			{
				CreateMul(tempRegister, arrayElementValue->GetIndex()[i]->GetShared());
			}
			CreateAdd(arrayPointer, tempRegister);
		}

		return arrayPointer;
	}

	return nullptr;
}

Share<HazeCompilerValue> HazeCompiler::CreatePointerToPointerArray(Share<HazeCompilerValue> pointerArray,
	Share<HazeCompilerValue> index)
{
	auto pointer = GetTempRegister();

	auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
	type.PrimaryType = HazeValueType::PointerBase;

	auto ret = CreateMov(pointer, pointerArray);
	if (index)
	{
		CreateAdd(ret, index);
	}

	return ret;
}

Share<HazeCompilerValue> HazeCompiler::CreatePointerToFunction(Share<HazeCompilerFunction> function)
{
	auto pointer = GetTempRegister();
	auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
	type.PrimaryType = HazeValueType::PointerFunction;
	type.CustomName = function->GetName();
	//pointer->SetDataDesc(HazeDataDesc::FunctionAddress);

	return CreateLea(pointer, CreateVariable(GetCurrModule().get(), HazeDefineVariable({ HazeValueType::PointerFunction, function->GetName() },
		H_TEXT("")), HazeVariableScope::Ignore, HazeDataDesc::FunctionAddress, 0));
}

Share<HazeCompilerValue> HazeCompiler::CreateNew(Share<HazeCompilerFunction> function, const HazeDefineType& data,
	Share<HazeCompilerValue> countValue)
{
	return function->CreateNew(data, countValue);
}

Share<HazeCompilerValue> HazeCompiler::CreateCast(const HazeDefineType& type, Share<HazeCompilerValue> value)
{
	auto reg = GetTempRegister();
	auto& valueType1 = const_cast<HazeDefineType&>(reg->GetValueType());
	valueType1 = type;

	GetCurrModule()->GenIRCode_BinaryOperater(reg, value, InstructionOpCode::CVT);

	return reg;
}

Share<HazeCompilerValue> HazeCompiler::CreateCVT(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right)
{
	auto tempReg = GetTempRegister();
	tempReg->StoreValueType(left);
	GetCurrModule()->GenIRCode_BinaryOperater(tempReg, right, InstructionOpCode::CVT);

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

	GetCurrModule()->GenIRCode_BinaryOperater(left, right, InstructionOpCode::CMP);
	return GetRegister(CMP_REGISTER);
}

Share<HazeCompilerValue> HazeCompiler::CreateBoolCmp(Share<HazeCompilerValue> value)
{
	if (IsConstantValueBoolTrue(value))
	{
		CreateIntCmp(CreateBitXor(CreateMov(GetTempRegister(), GenConstantValueBool(true)), GenConstantValueBool(false)), GenConstantValueBool(true));
		return GenConstantValueBool(true);
	}
	else if (IsConstantValueBoolFalse(value))
	{
		CreateIntCmp(CreateBitXor(CreateMov(GetTempRegister(), GenConstantValueBool(true)), GenConstantValueBool(false)), GenConstantValueBool(false));
		return GenConstantValueBool(false);
	}
	else
	{
		return CreateIntCmp(value, GenConstantValueBool(true));
	}
}

void HazeCompiler::ReplaceConstantValueByStrongerType(Share<HazeCompilerValue>& left, Share<HazeCompilerValue>& right)
{
	if (IsPointerType(left->GetValueType().PrimaryType) || IsPointerType(right->GetValueType().PrimaryType)
		|| (IsArrayType(left->GetValueType().PrimaryType) && IsNumberType(right->GetValueType().PrimaryType)))
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
				auto tempRegister = GetTempRegister();
				tempRegister->StoreValueType(left);
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
				auto tempRegister = GetTempRegister();
				tempRegister->StoreValueType(right);
				left = CreateMov(tempRegister, left, false);
			}
		}
	}
	
}

void HazeCompiler::CreateCompareJmp(HazeCmpType cmpType, Share<HazeBaseBlock> ifJmpBlock, Share<HazeBaseBlock> elseJmpBlock)
{
	GetCurrModule()->GenIRCode_Cmp(cmpType, ifJmpBlock, elseJmpBlock);
}