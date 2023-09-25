#include <filesystem>

#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeFilePathHelper.h"

#include "HazeCompiler.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerPointerArray.h"
#include "HazeCompilerInitListValue.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerHelper.h"

static std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> g_GlobalRegisters = {
	{ RET_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Ret_Register")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterRet, 0) },
	{ NEW_REGISTER, nullptr },
	{ CMP_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Cmp_Register")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterCmp, 0) },
};

static std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> g_GlobalTempRegisters = {
	{ TEMP_REGISTER_0, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register0")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_1, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register1")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_2, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register2")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_3, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register3")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_4, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register4")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_5, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register5")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_6, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register6")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_7, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register7")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_8, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register8")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_9, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register9")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0) },
};

static std::shared_ptr<HazeCompilerInitListValue> s_InitializeListValue =
std::make_shared<HazeCompilerInitListValue>(nullptr, HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register")), HazeVariableScope::Global, HazeDataDesc::Initlist, 0);

HazeCompiler::HazeCompiler(HazeVM* vm) : m_VM(vm)
{
}

HazeCompiler::~HazeCompiler()
{
}

bool HazeCompiler::InitializeCompiler(const HAZE_STRING& moduleName)
{
	//生成模块
	auto It = m_CompilerModules.find(moduleName);
	if (It != m_CompilerModules.end())
	{
		return false;
	}

	m_ModuleNameStack.push_back(moduleName);
	m_CompilerModules[GetCurrModuleName()] = std::make_unique<HazeCompilerModule>(this, moduleName);
	return true;
}

HazeCompilerModule* HazeCompiler::ParseModule(const HAZE_STRING& moduleName)
{
	m_VM->ParseFile(GetModuleFilePath(moduleName), moduleName);
	return GetModule(moduleName);
}

void HazeCompiler::FinishModule()
{
	m_CompilerModules[GetCurrModuleName()]->FinishModule();
}

HazeCompilerModule* HazeCompiler::GetModule(const HAZE_STRING& name)
{
	auto Iter = m_CompilerModules.find(name);
	if (Iter != m_CompilerModules.end())
	{
		return Iter->second.get();
	}

	return nullptr;
}

const HAZE_STRING* HazeCompiler::GetModuleName(const HazeCompilerModule* compilerModule) const
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

void HazeCompiler::InsertLineCount(int64 lineCount)
{
	if (m_VM->IsDebug() && m_InsertBaseBlock)
	{
		HAZE_LOG_INFO("HazeCompiler line %d\n", lineCount);
		m_InsertBaseBlock->PushIRCode(GetInstructionString(InstructionOpCode::LINE) + (HAZE_TEXT(" ") + HAZE_TO_HAZE_STR(lineCount)) + HAZE_TEXT("\n"));
	}
}

bool HazeCompiler::IsDebug() const
{
	return m_VM->IsDebug();
}

std::unique_ptr<HazeCompilerModule>& HazeCompiler::GetCurrModule()
{
	return m_CompilerModules[GetCurrModuleName()];
}

bool HazeCompiler::CurrModuleIsStdLib()
{
	return GetCurrModule()->GetModuleLibraryType() == HazeLibraryType::Standard;
}

std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> HazeCompiler::GetFunction(const HAZE_STRING& name)
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

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetNewRegister(HazeCompilerModule* compilerModule, const HazeDefineType& data)
{
	auto iter = g_GlobalRegisters.find(NEW_REGISTER);
	if (iter != g_GlobalRegisters.end())
	{
		iter->second = CreateVariable(compilerModule, HazeDefineVariable(data, HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterNew, 0);

		return iter->second;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetTempRegister()
{
	for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second.use_count() == 1)
		{
			return iter.second;
		}
	}

	HAZE_LOG_ERR(HAZE_TEXT("不能获得临时寄存器!\n"));
	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetRegister(const HAZE_CHAR* name)
{
	auto iter = g_GlobalRegisters.find(name);
	if (iter != g_GlobalRegisters.end())
	{
		return iter->second;
	}

	return nullptr;
}

const HAZE_CHAR* HazeCompiler::GetRegisterName(const std::shared_ptr<HazeCompilerValue>& compilerRegister)
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

	HAZE_LOG_ERR(HAZE_TEXT("查找寄存器错误,未能找到!\n"));
	return nullptr;
}

std::shared_ptr<HazeCompilerInitListValue> HazeCompiler::GetInitializeListValue()
{
	return s_InitializeListValue;
}

bool HazeCompiler::IsClass(const HAZE_STRING& name)
{
	auto& currModule = GetCurrModule();
	if (currModule->GetClass(name))
	{
		return true;
	}
	else
	{
		for (auto& Iter : currModule->m_Vector_ImportModules)
		{
			if (Iter->GetClass(name))
			{
				return true;
			}
		}
	}

	return false;
}

const HAZE_CHAR* HazeCompiler::GetClassName(const HAZE_STRING& name)
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

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenConstantValue(HazeValueType type, const HazeValue& var)
{
	static HazeDefineVariable s_DefineVariable;

	HazeValue& v = const_cast<HazeValue&>(var);
	s_DefineVariable.m_Type.PrimaryType = type;

	std::shared_ptr<HazeCompilerValue> ret = nullptr;
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
		HAZE_LOG_ERR(HAZE_TEXT("未支持生成<%s>常量类型!\n"), GetHazeValueTypeString(type));
		break;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenStringVariable(HAZE_STRING& str)
{
	return GetCurrModule()->GetOrCreateGlobalStringVariable(str);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetGlobalVariable(const HAZE_STRING& name)
{
	return GetCurrModule()->GetGlobalVariable(name);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetLocalVariable(const HAZE_STRING& name)
{
	return  GetCurrModule()->GetCurrFunction()->GetLocalVariable(name);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetConstantValueInt(int v)
{
	HazeValue Value;
	Value.Value.Int = v;

	return GenConstantValue(HazeValueType::Int, Value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenConstantValueBool(bool isTrue)
{
	HazeValue Value;
	Value.Value.Bool = isTrue;

	return GenConstantValue(HazeValueType::Bool, Value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetNullPtr(const HazeDefineType& type)
{
	static std::unordered_map<HazeDefineType, std::shared_ptr<HazeCompilerValue>, HazeDefineTypeHashFunction> s_NullPtrs;

	auto Iter = s_NullPtrs.find(type);
	if (Iter != s_NullPtrs.end())
	{
		return Iter->second;
	}

	s_NullPtrs[type] = CreateVariable(nullptr, HazeDefineVariable(type, HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::NullPtr, 0);
	return s_NullPtrs[type];
}

bool HazeCompiler::IsConstantValueBoolTrue(std::shared_ptr<HazeCompilerValue> v)
{
	return v == GenConstantValueBool(true);
}

bool HazeCompiler::IsConstantValueBoolFalse(std::shared_ptr<HazeCompilerValue> v)
{
	return v == GenConstantValueBool(false);
}

void HazeCompiler::SetInsertBlock(std::shared_ptr<HazeBaseBlock> block)
{
	m_InsertBaseBlock = block;
}

void HazeCompiler::ClearBlockPoint()
{
	m_InsertBaseBlock = nullptr;
}

void HazeCompiler::AddImportModuleToCurrModule(HazeCompilerModule* compilerModule)
{
	for (auto& iter : GetCurrModule()->m_Vector_ImportModules)
	{
		if (iter == compilerModule)
		{
			return;
		}
	}

	GetCurrModule()->m_Vector_ImportModules.push_back(compilerModule);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateLea(std::shared_ptr<HazeCompilerValue> allocaValue, std::shared_ptr<HazeCompilerValue> value)
{
	return GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, InstructionOpCode::LEA);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMov(std::shared_ptr<HazeCompilerValue> allocaValue, std::shared_ptr<HazeCompilerValue> value)
{
	allocaValue->StoreValueType(value);

	if ((allocaValue->IsPointer() && !value->IsPointer()) || (allocaValue->IsPointerPointer() && value->IsPointer()))
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
		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, InstructionOpCode::MOV);
	}

	return allocaValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMovToPV(std::shared_ptr<HazeCompilerValue> allocaValue, std::shared_ptr<HazeCompilerValue> value)
{
	if (allocaValue->IsRef() || allocaValue->IsPointer())
	{
		return GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, InstructionOpCode::MOVTOPV);
	}

	return allocaValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMovPV(std::shared_ptr<HazeCompilerValue> allocaValue, std::shared_ptr<HazeCompilerValue> value)
{
	if (value->IsPointer() || value->IsRef())
	{
		allocaValue->StoreValueType(value);

		HazeDefineType& allocaValueType = const_cast<HazeDefineType&>(allocaValue->GetValueType());
		auto& ValueValueType = value->GetValueType();

		allocaValueType.PrimaryType = ValueValueType.SecondaryType;
		allocaValueType.CustomName = ValueValueType.CustomName;

		allocaValueType.SecondaryType = HazeValueType::Void;

		return GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, InstructionOpCode::MOVPV);
	}
	else if (value->IsArray())
	{
		return CreateArrayElement(value, { GetConstantValueInt(0) });
	}

	return allocaValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> function, const HazeDefineVariable& variable, int line,
	std::shared_ptr<HazeCompilerValue> refValue, std::vector<std::shared_ptr<HazeCompilerValue>> arraySize, std::vector<HazeDefineType>* params)
{
	return function->CreateLocalVariable(variable, line, refValue, arraySize, params);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& compilerModule, const HazeDefineVariable& var,
	std::shared_ptr<HazeCompilerValue> refValue, std::vector<std::shared_ptr<HazeCompilerValue>> arraySize, std::vector<HazeDefineType>* params)
{
	return compilerModule->CreateGlobalVariable(var, refValue, arraySize, params);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateClassVariable(std::unique_ptr<HazeCompilerModule>& compilerModule, const HazeDefineVariable& var,
	std::shared_ptr<HazeCompilerValue> refValue, std::vector<std::shared_ptr<HazeCompilerValue>> arraySize, std::vector<HazeDefineType>* params)
{
	return CreateVariable(compilerModule.get(), var, HazeVariableScope::None, HazeDataDesc::Class, 0, refValue, arraySize, params);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateRet(std::shared_ptr<HazeCompilerValue> value)
{
	GetCurrModule()->GenIRCode_Ret(value);
	return value;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateAdd(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateAdd(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateSub(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateSub(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMul(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateMul(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateDiv(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateDiv(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMod(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateMod(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBitAnd(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateBitAnd(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBitOr(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateBitOr(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBitNeg(std::shared_ptr<HazeCompilerValue> value)
{
	return GetCurrModule()->CreateBitNeg(value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBitXor(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateBitXor(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateShl(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateShl(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateShr(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign)
{
	return GetCurrModule()->CreateShr(left, right, isAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateNot(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right)
{
	return GetCurrModule()->CreateNot(left, right);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> function, std::vector<std::shared_ptr<HazeCompilerValue>>& param, std::shared_ptr<HazeCompilerValue> thisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(function, param, thisPointerTo);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateFunctionCall(std::shared_ptr<HazeCompilerValue> pointerFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& param, std::shared_ptr<HazeCompilerValue> thisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(pointerFunction, param, thisPointerTo);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateArrayInit(std::shared_ptr<HazeCompilerValue> arrValue, std::shared_ptr<HazeCompilerValue> initList)
{
	return GetCurrModule()->CreateArrayInit(arrValue, initList);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateArrayElement(std::shared_ptr<HazeCompilerValue> value, std::vector<uint32> index)
{
	std::vector<std::shared_ptr<HazeCompilerValue>> indces;
	for (size_t i = 0; i < index.size(); i++)
	{
		HazeValue IndexValue;
		IndexValue.Value.Int = index[i];

		indces.push_back(GenConstantValue(HazeValueType::Int, IndexValue));
	}

	return CreateArrayElement(value, indces);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateArrayElement(std::shared_ptr<HazeCompilerValue> value, std::vector<std::shared_ptr<HazeCompilerValue>> indices)
{
	std::vector<HazeCompilerValue*> values;
	for (auto& Iter : indices)
	{
		values.push_back(Iter.get());
	}

	if (value->IsArray())
	{
		auto arrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(value);
		return std::make_shared<HazeCompilerArrayElementValue>(GetCurrModule().get(), HazeDefineType(arrayValue->GetValueType().SecondaryType,
			arrayValue->GetValueType().CustomName), value->GetVariableScope(), HazeDataDesc::ArrayElement, 0, value.get(), values);
	}
	else if (value->IsPointer())
	{
		auto pointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(value);
		if (pointerValue)
		{
			return std::make_shared<HazeCompilerArrayElementValue>(GetCurrModule().get(), HazeDefineType(pointerValue->GetValueType().SecondaryType,
				pointerValue->GetValueType().CustomName), value->GetVariableScope(), HazeDataDesc::ArrayElement, 0, value.get(), values);
			//return CreateMovPV(GetTempRegister(), CreateAdd(CreateMov(GetTempRegister(), Value), Index));
		}
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToValue(std::shared_ptr<HazeCompilerValue> value)
{
	auto tempReg = GetTempRegister();
	tempReg->StoreValueType(value);

	auto& type = const_cast<HazeDefineType&>(tempReg->GetValueType());
	type.SecondaryType = type.PrimaryType;

	if (IsHazeDefaultType(type.SecondaryType))
	{
		type.PrimaryType = HazeValueType::PointerBase;
	}
	else if (type.SecondaryType == HazeValueType::Class)
	{
		type.PrimaryType = HazeValueType::PointerClass;
	}

	return CreateLea(tempReg, value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToArray(std::shared_ptr<HazeCompilerValue> arrValue, std::shared_ptr<HazeCompilerValue> index)
{
	auto pointer = GetTempRegister();

	auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
	type.PrimaryType = HazeValueType::PointerBase;
	type.SecondaryType = arrValue->GetValueType().SecondaryType;
	type.CustomName = arrValue->GetValueType().CustomName;

	auto ret = CreateMov(pointer, arrValue);
	if (index)
	{
		CreateAdd(ret, index);
	}

	return ret;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToArrayElement(std::shared_ptr<HazeCompilerValue> elementValue)
{
	auto arrayElementValue = std::dynamic_pointer_cast<HazeCompilerArrayElementValue>(elementValue);
	auto arrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(arrayElementValue->GetArray()->GetShared());
	auto pointerValue = std::dynamic_pointer_cast<HazeCompilerPointerArray>(arrayElementValue->GetArray()->GetShared());

	if (arrayValue || pointerValue)
	{
		auto tempRegister = GetTempRegister();
		std::shared_ptr<HazeCompilerValue> arrayPointer;
		if (arrayValue)
		{
			arrayPointer = CreatePointerToArray(arrayValue);
		}
		else
		{
			arrayPointer = CreatePointerToPointerArray(pointerValue);
		}

		for (size_t i = 0; i < arrayElementValue->GetIndex().size(); i++)
		{
			uint32 size = i == arrayElementValue->GetIndex().size() - 1 ? arrayElementValue->GetIndex()[i]->GetValue().Value.Int :
				(arrayValue ? arrayValue->GetSizeByLevel((uint32)i) : pointerValue->GetSizeByLevel((uint32)i));
			std::shared_ptr<HazeCompilerValue> sizeValue = nullptr;

			if (arrayElementValue->GetIndex()[i]->IsConstant())
			{
				sizeValue = GetConstantValueInt(size);
			}
			else
			{
				sizeValue = i == arrayElementValue->GetIndex().size() - 1 ? arrayElementValue->GetIndex()[i]->GetShared() : GetConstantValueInt(size);
			}

			CreateMov(tempRegister, sizeValue);
			if (i != arrayElementValue->GetIndex().size() - 1)
			{
				CreateMul(tempRegister, arrayElementValue->GetIndex()[i]->GetShared());
			}
			CreateAdd(arrayPointer, tempRegister);
		}

		return arrayPointer;
		//Compiler->CreateMovPV(MovToValue ? MovToValue : TempRegister, ArrayPointer);
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToPointerArray(std::shared_ptr<HazeCompilerValue> pointerArray,
	std::shared_ptr<HazeCompilerValue> index)
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

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToFunction(std::shared_ptr<HazeCompilerFunction> function)
{
	return CreateVariable(GetCurrModule().get(), HazeDefineVariable({ HazeValueType::PointerFunction, function->GetName() },
		HAZE_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::FunctionAddress, 0);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateNew(std::shared_ptr<HazeCompilerFunction> function, const HazeDefineType& data)
{
	return function->CreateNew(data);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateNeg(std::shared_ptr<HazeCompilerValue> value)
{
	return GetCurrModule()->CreateNeg(value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateInc(std::shared_ptr<HazeCompilerValue> value, bool isPreInc)
{
	return GetCurrModule()->CreateInc(value, isPreInc);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateDec(std::shared_ptr<HazeCompilerValue> value, bool isPreDec)
{
	return GetCurrModule()->CreateDec(value, isPreDec);
}

void HazeCompiler::CreateJmpToBlock(std::shared_ptr<HazeBaseBlock> block)
{
	GetCurrModule()->GenIRCode_JmpTo(block);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateIntCmp(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right)
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

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBoolCmp(std::shared_ptr<HazeCompilerValue> value)
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

void HazeCompiler::CreateCompareJmp(HazeCmpType cmpType, std::shared_ptr<HazeBaseBlock> ifJmpBlock, std::shared_ptr<HazeBaseBlock> elseJmpBlock)
{
	GetCurrModule()->GenIRCode_Cmp(cmpType, ifJmpBlock, elseJmpBlock);
}