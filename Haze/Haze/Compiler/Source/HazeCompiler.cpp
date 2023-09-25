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

static std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> HashMap_GlobalRegister = {
	{ RET_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Ret_Register")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterRet, 0) },
	{ NEW_REGISTER, nullptr },
	{ CMP_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Cmp_Register")), HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterCmp, 0) },
};

static std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> HashMap_GlobalTempRegister = {
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

static std::shared_ptr<HazeCompilerInitListValue> InitializeListValue =
std::make_shared<HazeCompilerInitListValue>(nullptr, HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register")), HazeVariableScope::Global, HazeDataDesc::Initlist, 0);

HazeCompiler::HazeCompiler(HazeVM* m_VM) : m_VM(m_VM)
{
}

HazeCompiler::~HazeCompiler()
{
}

bool HazeCompiler::InitializeCompiler(const HAZE_STRING& m_ModuleName)
{
	//生成模块
	auto It = HashMap_CompilerModule.find(m_ModuleName);
	if (It != HashMap_CompilerModule.end())
	{
		return false;
	}

	Vector_ModuleNameStack.push_back(m_ModuleName);
	HashMap_CompilerModule[GetCurrModuleName()] = std::make_unique<HazeCompilerModule>(this, m_ModuleName);
	return true;
}

HazeCompilerModule* HazeCompiler::ParseModule(const HAZE_STRING& m_ModuleName)
{
	m_VM->ParseFile(GetModuleFilePath(m_ModuleName), m_ModuleName);
	return GetModule(m_ModuleName);
}

void HazeCompiler::FinishModule()
{
	HashMap_CompilerModule[GetCurrModuleName()]->FinishModule();
}

HazeCompilerModule* HazeCompiler::GetModule(const HAZE_STRING& m_Name)
{
	auto Iter = HashMap_CompilerModule.find(m_Name);
	if (Iter != HashMap_CompilerModule.end())
	{
		return Iter->second.get();
	}

	return nullptr;
}

const HAZE_STRING* HazeCompiler::GetModuleName(const HazeCompilerModule* m_Module) const
{
	for (auto& Iter : HashMap_CompilerModule)
	{
		if (Iter.second.get() == m_Module)
		{
			return &(Iter.first);
		}
	}

	return nullptr;
}

void HazeCompiler::InsertLineCount(int64 LineCount)
{
	if (m_VM->IsDebug() && InsertBaseBlock)
	{
		HAZE_LOG_INFO("HazeCompiler line %d\n", LineCount);
		InsertBaseBlock->PushIRCode(GetInstructionString(InstructionOpCode::LINE) + (HAZE_TEXT(" ") + HAZE_TO_HAZE_STR(LineCount)) + HAZE_TEXT("\n"));
	}
}

bool HazeCompiler::IsDebug() const
{
	return m_VM->IsDebug();
}

std::unique_ptr<HazeCompilerModule>& HazeCompiler::GetCurrModule()
{
	return HashMap_CompilerModule[GetCurrModuleName()];
}

bool HazeCompiler::CurrModuleIsStdLib()
{
	return GetCurrModule()->GetModuleLibraryType() == HazeLibraryType::Standard;
}

std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> HazeCompiler::GetFunction(const HAZE_STRING& m_Name)
{
	for (auto& Iter : HashMap_CompilerModule)
	{
		auto Function = Iter.second->GetFunction(m_Name);
		if (Function.first)
		{
			return Function;
		}
	}

	return { nullptr, nullptr };
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetNewRegister(HazeCompilerModule* m_Module, const HazeDefineType& m_Data)
{
	auto Iter = HashMap_GlobalRegister.find(NEW_REGISTER);
	if (Iter != HashMap_GlobalRegister.end())
	{
		Iter->second = CreateVariable(m_Module, HazeDefineVariable(m_Data, HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::RegisterNew, 0);

		return Iter->second;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetTempRegister()
{
	for (auto& Iter : HashMap_GlobalTempRegister)
	{
		if (Iter.second.use_count() == 1)
		{
			return Iter.second;
		}
	}

	HAZE_LOG_ERR(HAZE_TEXT("不能获得临时寄存器!\n"));
	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetRegister(const HAZE_CHAR* m_Name)
{
	auto Iter = HashMap_GlobalRegister.find(m_Name);
	if (Iter != HashMap_GlobalRegister.end())
	{
		return Iter->second;
	}

	return nullptr;
}

const HAZE_CHAR* HazeCompiler::GetRegisterName(const std::shared_ptr<HazeCompilerValue>& Register)
{
	for (auto& Iter : HashMap_GlobalRegister)
	{
		if (Iter.second == Register)
		{
			return Iter.first;
		}
	}

	for (auto& Iter : HashMap_GlobalTempRegister)
	{
		if (Iter.second == Register)
		{
			return Iter.first;
		}
	}

	HAZE_LOG_ERR(HAZE_TEXT("查找寄存器错误,未能找到!\n"));
	return nullptr;
}

std::shared_ptr<HazeCompilerInitListValue> HazeCompiler::GetInitializeListValue()
{
	return InitializeListValue;
}

bool HazeCompiler::IsClass(const HAZE_STRING& m_Name)
{
	auto& m_Module = GetCurrModule();
	if (m_Module->GetClass(m_Name))
	{
		return true;
	}
	else
	{
		for (auto& Iter : m_Module->m_Vector_ImportModules)
		{
			if (Iter->GetClass(m_Name))
			{
				return true;
			}
		}
	}

	return false;
}

const HAZE_CHAR* HazeCompiler::GetClassName(const HAZE_STRING& m_Name)
{
	for (auto& Iter : HashMap_CompilerModule)
	{
		auto Class = Iter.second->GetClass(m_Name);
		if (Class)
		{
			return Class->GetName().c_str();
		}
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenConstantValue(HazeValueType m_Type, const HazeValue& Var)
{
	HazeValue& V = const_cast<HazeValue&>(Var);
	static HazeDefineVariable m_DefineVariable;
	m_DefineVariable.m_Type.PrimaryType = m_Type;

	std::shared_ptr<HazeCompilerValue> Ret = nullptr;
	switch (m_Type)
	{
	case HazeValueType::Bool:
	{
		auto it = HashMap_BoolConstantValue.find(Var.Value.Bool);
		if (it != HashMap_BoolConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_BoolConstantValue[Var.Value.Bool] = Ret;
		return Ret;
	}
	case HazeValueType::Byte:
	{
		auto it = HashMap_ByteConstantValue.find(Var.Value.Byte);
		if (it != HashMap_ByteConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_ByteConstantValue[Var.Value.Byte] = Ret;
		return Ret;
	}
	case HazeValueType::UnsignedByte:
	{
		auto it = HashMap_UnsignedByteConstantValue.find(Var.Value.UnsignedByte);
		if (it != HashMap_UnsignedByteConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_UnsignedByteConstantValue[Var.Value.UnsignedByte] = Ret;
		return Ret;
	}
	case HazeValueType::Char:
	{
		auto it = HashMap_CharConstantValue.find(Var.Value.Char);
		if (it != HashMap_CharConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_CharConstantValue[Var.Value.Char] = Ret;
		return Ret;
	}
	case HazeValueType::Short:
	{
		auto it = HashMap_ShortConstantValue.find(Var.Value.Short);
		if (it != HashMap_ShortConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_ShortConstantValue[Var.Value.Short] = Ret;
		return Ret;
	}
	case HazeValueType::UnsignedShort:
	{
		auto it = HashMap_UnsignedShortConstantValue.find(Var.Value.UnsignedShort);
		if (it != HashMap_UnsignedShortConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_UnsignedShortConstantValue[Var.Value.UnsignedShort] = Ret;
		return HashMap_UnsignedShortConstantValue[Var.Value.UnsignedShort];
	}
	case HazeValueType::Int:
	{
		auto it = HashMap_IntConstantValue.find(Var.Value.Int);
		if (it != HashMap_IntConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_IntConstantValue[Var.Value.Int] = Ret;
		return HashMap_IntConstantValue[Var.Value.Int];
	}
	case HazeValueType::Long:
	{
		auto it = HashMap_LongConstantValue.find(Var.Value.Long);
		if (it != HashMap_LongConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_LongConstantValue[Var.Value.Long] = Ret;
		return Ret;
	}
	case HazeValueType::UnsignedInt:
	{
		auto it = HashMap_UnsignedIntConstantValue.find(Var.Value.UnsignedInt);
		if (it != HashMap_UnsignedIntConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_UnsignedIntConstantValue[Var.Value.UnsignedInt] = Ret;
		return Ret;
	}
	case HazeValueType::UnsignedLong:
	{
		auto it = HashMap_UnsignedLongConstantValue.find(Var.Value.UnsignedLong);
		if (it != HashMap_UnsignedLongConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_UnsignedLongConstantValue[Var.Value.UnsignedLong] = Ret;
		return Ret;
	}
	case HazeValueType::Float:
	{
		auto it = HashMap_FloatConstantValue.find(Var.Value.Float);
		if (it != HashMap_FloatConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_FloatConstantValue[Var.Value.Float] = Ret;
		return Ret;
	}
	case HazeValueType::Double:
	{
		auto it = HashMap_DobuleConstantValue.find(Var.Value.Double);
		if (it != HashMap_DobuleConstantValue.end())
		{
			return it->second;
		}
		Ret = CreateVariable(nullptr, m_DefineVariable, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		Ret->StoreValue(V);
		HashMap_DobuleConstantValue[Var.Value.Double] = Ret;
		return Ret;
	}
	default:
		HAZE_LOG_ERR(HAZE_TEXT("未支持生成<%s>常量类型!\n"), GetHazeValueTypeString(m_Type));
		break;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenStringVariable(HAZE_STRING& String)
{
	return GetCurrModule()->GetOrCreateGlobalStringVariable(String);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetGlobalVariable(const HAZE_STRING& m_Name)
{
	return GetCurrModule()->GetGlobalVariable(m_Name);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetLocalVariable(const HAZE_STRING& m_Name)
{
	return  GetCurrModule()->GetCurrFunction()->GetLocalVariable(m_Name);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetConstantValueInt(int V)
{
	HazeValue Value;
	Value.Value.Int = V;

	return GenConstantValue(HazeValueType::Int, Value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenConstantValueBool(bool IsTrue)
{
	HazeValue Value;
	Value.Value.Bool = IsTrue;

	return GenConstantValue(HazeValueType::Bool, Value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetNullPtr(const HazeDefineType& m_Type)
{
	static std::unordered_map<HazeDefineType, std::shared_ptr<HazeCompilerValue>, HazeDefineTypeHashFunction> HashMap_NullPtr;

	auto Iter = HashMap_NullPtr.find(m_Type);
	if (Iter != HashMap_NullPtr.end())
	{
		return Iter->second;
	}

	HashMap_NullPtr[m_Type] = CreateVariable(nullptr, HazeDefineVariable(m_Type, HAZE_TEXT("")), HazeVariableScope::Global, HazeDataDesc::NullPtr, 0);
	return HashMap_NullPtr[m_Type];
}

bool HazeCompiler::IsConstantValueBoolTrue(std::shared_ptr<HazeCompilerValue> V)
{
	return V == GenConstantValueBool(true);
}

bool HazeCompiler::IsConstantValueBoolFalse(std::shared_ptr<HazeCompilerValue> V)
{
	return V == GenConstantValueBool(false);
}

void HazeCompiler::SetInsertBlock(std::shared_ptr<HazeBaseBlock> BB)
{
	InsertBaseBlock = BB;
}

void HazeCompiler::ClearBlockPoint()
{
	InsertBaseBlock = nullptr;
}

void HazeCompiler::AddImportModuleToCurrModule(HazeCompilerModule* m_Module)
{
	for (auto& Iter : GetCurrModule()->m_Vector_ImportModules)
	{
		if (Iter == m_Module)
		{
			return;
		}
	}

	GetCurrModule()->m_Vector_ImportModules.push_back(m_Module);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateLea(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value)
{
	return GetCurrModule()->GenIRCode_BinaryOperater(Alloca, Value, InstructionOpCode::LEA);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMov(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value)
{
	Alloca->StoreValueType(Value);

	if ((Alloca->IsPointer() && !Value->IsPointer()) || (Alloca->IsPointerPointer() && Value->IsPointer()))
	{
		CreateLea(Alloca, Value);
	}
	else if (Alloca->IsRef())
	{
		CreateMovToPV(Alloca, Value->IsRef() ? CreateMovPV(GetTempRegister(), Value) : Value);
	}
	else if (Value->IsArrayElement() || Alloca->IsArrayElement())
	{
		if (Alloca->IsArrayElement() && !Value->IsArrayElement())
		{
			auto ArrayPointer = CreatePointerToArrayElement(Alloca);
			CreateMovToPV(ArrayPointer, Value);
		}
		else if (!Alloca->IsArrayElement() && Value->IsArrayElement())
		{
			GetArrayElementToValue(GetCurrModule().get(), Value, Alloca);
		}
		else
		{
			auto TempValue = GetArrayElementToValue(GetCurrModule().get(), Value);
			auto ArrayPointer = CreatePointerToArrayElement(Alloca);
			CreateMovToPV(ArrayPointer, TempValue);
		}
	}
	else
	{
		GetCurrModule()->GenIRCode_BinaryOperater(Alloca, Value, InstructionOpCode::MOV);
	}

	return Alloca;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMovToPV(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value)
{
	if (Alloca->IsRef() || Alloca->IsPointer())
	{
		return GetCurrModule()->GenIRCode_BinaryOperater(Alloca, Value, InstructionOpCode::MOVTOPV);
	}

	return Alloca;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMovPV(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value)
{
	if (Value->IsPointer() || Value->IsRef())
	{
		Alloca->StoreValueType(Value);

		HazeDefineType& AllocaValueType = const_cast<HazeDefineType&>(Alloca->GetValueType());
		auto& ValueValueType = Value->GetValueType();

		AllocaValueType.PrimaryType = ValueValueType.SecondaryType;
		AllocaValueType.CustomName = ValueValueType.CustomName;

		AllocaValueType.SecondaryType = HazeValueType::Void;

		return GetCurrModule()->GenIRCode_BinaryOperater(Alloca, Value, InstructionOpCode::MOVPV);
	}
	else if (Value->IsArray())
	{
		return CreateArrayElement(Value, { GetConstantValueInt(0) });
	}

	return Alloca;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineVariable& Variable, int Line,
	std::shared_ptr<HazeCompilerValue> RefValue, std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize, std::vector<HazeDefineType>* Params)
{
	return Function->CreateLocalVariable(Variable, Line, RefValue, m_ArraySize, Params);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& m_Module, const HazeDefineVariable& Var,
	std::shared_ptr<HazeCompilerValue> RefValue, std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize, std::vector<HazeDefineType>* Params)
{
	return m_Module->CreateGlobalVariable(Var, RefValue, m_ArraySize, Params);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateClassVariable(std::unique_ptr<HazeCompilerModule>& m_Module, const HazeDefineVariable& Var,
	std::shared_ptr<HazeCompilerValue> RefValue, std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize, std::vector<HazeDefineType>* Params)
{
	return CreateVariable(m_Module.get(), Var, HazeVariableScope::None, HazeDataDesc::Class, 0, RefValue, m_ArraySize, Params);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateRet(std::shared_ptr<HazeCompilerValue> Value)
{
	GetCurrModule()->GenIRCode_Ret(Value);
	return Value;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateAdd(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateSub(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateSub(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMul(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateMul(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateDiv(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateDiv(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMod(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateMod(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBitAnd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateBitAnd(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBitOr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateBitOr(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBitNeg(std::shared_ptr<HazeCompilerValue> Value)
{
	return GetCurrModule()->CreateBitNeg(Value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBitXor(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateBitXor(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateShl(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateShl(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateShr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GetCurrModule()->CreateShr(Left, Right, IsAssign);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateNot(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GetCurrModule()->CreateNot(Left, Right);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(Function, Param, ThisPointerTo);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateFunctionCall(std::shared_ptr<HazeCompilerValue> PointerFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(PointerFunction, Param, ThisPointerTo);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateArrayInit(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> InitList)
{
	return GetCurrModule()->CreateArrayInit(Array, InitList);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateArrayElement(std::shared_ptr<HazeCompilerValue> Value, std::vector<uint32> Index)
{
	std::vector<std::shared_ptr<HazeCompilerValue>> Vector_Index;
	for (size_t i = 0; i < Index.size(); i++)
	{
		HazeValue IndexValue;
		IndexValue.Value.Int = Index[i];

		Vector_Index.push_back(GenConstantValue(HazeValueType::Int, IndexValue));
	}

	return CreateArrayElement(Value, Vector_Index);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateArrayElement(std::shared_ptr<HazeCompilerValue> Value, std::vector<std::shared_ptr<HazeCompilerValue>> Index)
{
	std::vector<HazeCompilerValue*> Vector_index;
	for (auto& Iter : Index)
	{
		Vector_index.push_back(Iter.get());
	}

	if (Value->IsArray())
	{
		auto ArrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(Value);
		return std::make_shared<HazeCompilerArrayElementValue>(GetCurrModule().get(), HazeDefineType(ArrayValue->GetValueType().SecondaryType,
			ArrayValue->GetValueType().CustomName), Value->GetVariableScope(), HazeDataDesc::ArrayElement, 0, Value.get(), Vector_index);
	}
	else if (Value->IsPointer())
	{
		auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
		if (PointerValue)
		{
			return std::make_shared<HazeCompilerArrayElementValue>(GetCurrModule().get(), HazeDefineType(PointerValue->GetValueType().SecondaryType,
				PointerValue->GetValueType().CustomName), Value->GetVariableScope(), HazeDataDesc::ArrayElement, 0, Value.get(), Vector_index);
			//return CreateMovPV(GetTempRegister(), CreateAdd(CreateMov(GetTempRegister(), Value), Index));
		}
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToValue(std::shared_ptr<HazeCompilerValue> Value)
{
	auto TempReg = GetTempRegister();
	TempReg->StoreValueType(Value);

	auto& m_Type = const_cast<HazeDefineType&>(TempReg->GetValueType());
	m_Type.SecondaryType = m_Type.PrimaryType;

	if (IsHazeDefaultType(m_Type.SecondaryType))
	{
		m_Type.PrimaryType = HazeValueType::PointerBase;
	}
	else if (m_Type.SecondaryType == HazeValueType::Class)
	{
		m_Type.PrimaryType = HazeValueType::PointerClass;
	}

	return CreateLea(TempReg, Value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToArray(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> Index)
{
	auto Pointer = GetTempRegister();

	auto& m_Type = const_cast<HazeDefineType&>(Pointer->GetValueType());
	m_Type.PrimaryType = HazeValueType::PointerBase;
	m_Type.SecondaryType = Array->GetValueType().SecondaryType;
	m_Type.CustomName = Array->GetValueType().CustomName;

	auto Ret = CreateMov(Pointer, Array);
	if (Index)
	{
		CreateAdd(Ret, Index);
	}

	return Ret;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToArrayElement(std::shared_ptr<HazeCompilerValue> ElementValue)
{
	auto ArrayElementValue = std::dynamic_pointer_cast<HazeCompilerArrayElementValue>(ElementValue);
	auto ArrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(ArrayElementValue->GetArray()->GetShared());
	auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerArray>(ArrayElementValue->GetArray()->GetShared());

	if (ArrayValue || PointerValue)
	{
		auto TempRegister = GetTempRegister();
		std::shared_ptr<HazeCompilerValue> ArrayPointer;
		if (ArrayValue)
		{
			ArrayPointer = CreatePointerToArray(ArrayValue);
		}
		else
		{
			ArrayPointer = CreatePointerToPointerArray(PointerValue);
		}

		for (size_t i = 0; i < ArrayElementValue->GetIndex().size(); i++)
		{
			uint32 Size = i == ArrayElementValue->GetIndex().size() - 1 ? ArrayElementValue->GetIndex()[i]->GetValue().Value.Int :
				(ArrayValue ? ArrayValue->GetSizeByLevel((uint32)i) : PointerValue->GetSizeByLevel((uint32)i));
			std::shared_ptr<HazeCompilerValue> SizeValue = nullptr;

			if (ArrayElementValue->GetIndex()[i]->IsConstant())
			{
				SizeValue = GetConstantValueInt(Size);
			}
			else
			{
				SizeValue = i == ArrayElementValue->GetIndex().size() - 1 ? ArrayElementValue->GetIndex()[i]->GetShared() : GetConstantValueInt(Size);
			}

			CreateMov(TempRegister, SizeValue);
			if (i != ArrayElementValue->GetIndex().size() - 1)
			{
				CreateMul(TempRegister, ArrayElementValue->GetIndex()[i]->GetShared());
			}
			CreateAdd(ArrayPointer, TempRegister);
		}

		return ArrayPointer;
		//Compiler->CreateMovPV(MovToValue ? MovToValue : TempRegister, ArrayPointer);
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToPointerArray(std::shared_ptr<HazeCompilerValue> PointerArray, std::shared_ptr<HazeCompilerValue> Index)
{
	auto Pointer = GetTempRegister();

	auto& m_Type = const_cast<HazeDefineType&>(Pointer->GetValueType());
	m_Type.PrimaryType = HazeValueType::PointerBase;

	auto Ret = CreateMov(Pointer, PointerArray);
	if (Index)
	{
		CreateAdd(Ret, Index);
	}

	return Ret;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreatePointerToFunction(std::shared_ptr<HazeCompilerFunction> Function)
{
	return CreateVariable(GetCurrModule().get(), HazeDefineVariable({ HazeValueType::PointerFunction, Function->GetName() }, HAZE_TEXT("")),
		HazeVariableScope::Temp, HazeDataDesc::FunctionAddress, 0);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateNew(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineType& m_Data)
{
	return Function->CreateNew(m_Data);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateNeg(std::shared_ptr<HazeCompilerValue> Value)
{
	return GetCurrModule()->CreateNeg(Value);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateInc(std::shared_ptr<HazeCompilerValue> Value, bool m_IsPreInc)
{
	return GetCurrModule()->CreateInc(Value, m_IsPreInc);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateDec(std::shared_ptr<HazeCompilerValue> Value, bool m_IsPreDec)
{
	return GetCurrModule()->CreateDec(Value, m_IsPreDec);
}

void HazeCompiler::CreateJmpToBlock(std::shared_ptr<HazeBaseBlock> Block)
{
	GetCurrModule()->GenIRCode_JmpTo(Block);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateIntCmp(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	if (Left->IsArrayElement())
	{
		Left = GetArrayElementToValue(GetCurrModule().get(), Left);
	}

	if (Right->IsArrayElement())
	{
		Right = GetArrayElementToValue(GetCurrModule().get(), Right);
	}

	GetCurrModule()->GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::CMP);
	return GetRegister(CMP_REGISTER);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateBoolCmp(std::shared_ptr<HazeCompilerValue> Value)
{
	if (IsConstantValueBoolTrue(Value))
	{
		CreateIntCmp(CreateBitXor(CreateMov(GetTempRegister(), GenConstantValueBool(true)), GenConstantValueBool(false)), GenConstantValueBool(true));
		return GenConstantValueBool(true);
	}
	else if (IsConstantValueBoolFalse(Value))
	{
		CreateIntCmp(CreateBitXor(CreateMov(GetTempRegister(), GenConstantValueBool(true)), GenConstantValueBool(false)), GenConstantValueBool(false));
		return GenConstantValueBool(false);
	}
	else
	{
		return CreateIntCmp(Value, GenConstantValueBool(true));
	}
}

void HazeCompiler::CreateCompareJmp(HazeCmpType CmpType, std::shared_ptr<HazeBaseBlock> IfJmpBlock, std::shared_ptr<HazeBaseBlock> ElseJmpBlock)
{
	GetCurrModule()->GenIRCode_Cmp(CmpType, IfJmpBlock, ElseJmpBlock);
}