#include <filesystem>

#include "HazeLog.h"
#include "HazeCompiler.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerInitListValue.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerHelper.h"

#include "HazeBaseBlock.h"

static std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> HashMap_GlobalRegister = {
	{ RET_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Ret_Register")), HAZE_TEXT("")), HazeDataDesc::RegisterRet, 0) },
	{ NEW_REGISTER, nullptr },
	{ CMP_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Cmp_Register")), HAZE_TEXT("")), HazeDataDesc::RegisterCmp, 0) },

	};

static std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> HashMap_GlobalTempRegister = {
	{ TEMP_REGISTER_1, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register1")), HAZE_TEXT("")), HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_2, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register2")), HAZE_TEXT("")), HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_3, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register3")), HAZE_TEXT("")), HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_4, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register4")), HAZE_TEXT("")), HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_5, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register5")), HAZE_TEXT("")), HazeDataDesc::RegisterTemp, 0) },
};

static std::shared_ptr<HazeCompilerInitListValue> InitializeListValue = 
std::make_shared<HazeCompilerInitListValue>(nullptr, HazeDefineType(HazeValueType::Void, HAZE_TEXT("Temp_Register")), HazeDataDesc::Initlist, 0);

HazeCompiler::HazeCompiler()
{
}

HazeCompiler::~HazeCompiler()
{
}

bool HazeCompiler::InitializeCompiler(const HAZE_STRING& ModuleName)
{
	//Éú³ÉÄ£¿é
	auto It = HashMap_CompilerModule.find(ModuleName);
	if (It != HashMap_CompilerModule.end())
	{
		return false;
	}

	Vector_ModuleNameStack.push_back(ModuleName);
	HashMap_CompilerModule[GetCurrModuleName()] = std::make_unique<HazeCompilerModule>(this, ModuleName);
	return true;
}

void HazeCompiler::FinishModule()
{
	GenModuleCodeFile();
}

HazeCompilerModule* HazeCompiler::GetModule(const HAZE_STRING& Name)
{
	auto Iter = HashMap_CompilerModule.find(Name);
	if (Iter != HashMap_CompilerModule.end())
	{
		return Iter->second.get();
	}

	return nullptr;
}

void HazeCompiler::GenModuleCodeFile()
{
	HashMap_CompilerModule[GetCurrModuleName()]->GenCodeFile();
}

std::unique_ptr<HazeCompilerModule>& HazeCompiler::GetCurrModule()
{
	return HashMap_CompilerModule[GetCurrModuleName()];
}

bool HazeCompiler::CurrModuleIsStdLib()
{
	return GetCurrModule()->IsStandardLibrary();
}

std::shared_ptr<HazeCompilerFunction> HazeCompiler::GetFunction(const HAZE_STRING& Name)
{
	std::shared_ptr<HazeCompilerFunction> Function = nullptr;
	for (auto& Iter : HashMap_CompilerModule)
	{
		Function = Iter.second->GetFunction(Name);
		if (Function)
		{
			return Function;
		}
	}

	return Function;
}

HAZE_STRING HazeCompiler::GetCurrModuleOpFile() const
{
	HAZE_STRING Path = std::filesystem::current_path();
	return Path + HAZE_TEXT("\\HazeOpCode\\") + GetCurrModuleName() + HAZE_TEXT(".Hzb");
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetNewRegister(HazeCompilerModule* Module, const HazeDefineType& Data)
{
	auto Iter = HashMap_GlobalRegister.find(NEW_REGISTER);
	if (Iter != HashMap_GlobalRegister.end())
	{
		Iter->second = CreateVariable(Module, HazeDefineVariable(Data, HAZE_TEXT("")), HazeDataDesc::RegisterNew, 0);

		auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Iter->second);
		
		if (!Data.CustomName.empty())
		{
			PointerValue->InitPointerTo(Module->FindClass(Data.CustomName)->GetNewPointerToValue());
		}
		
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

	HAZE_LOG_ERR(HAZE_TEXT("Get temp register error\n"));
	return nullptr;
}

//const HAZE_CHAR* HazeCompiler::GetRegisterName(std::shared_ptr<HazeCompilerValue> Register)
//{
//	for (auto& iter : Map_GlobalRegister)
//	{
//		if (iter.second == Register)
//		{
//			return iter.first;
//		}
//	}
//
//	return nullptr;
//}
//
std::shared_ptr<HazeCompilerValue> HazeCompiler::GetRegister(const HAZE_CHAR* Name)
{
	auto Iter = HashMap_GlobalRegister.find(Name);
	if (Iter != HashMap_GlobalRegister.end())
	{
		return Iter->second;
	}

	return nullptr;
}

const HAZE_CHAR* HazeCompiler::GetRegisterName(const std::shared_ptr<HazeCompilerValue>& Value)
{
	for (auto& Iter : HashMap_GlobalRegister)
	{
		if (Iter.second == Value)
		{
			return Iter.first;
		}
	}

	for (auto& Iter : HashMap_GlobalTempRegister)
	{
		if (Iter.second == Value)
		{
			return Iter.first;
		}
	}

	HAZE_LOG_ERR(HAZE_TEXT("find register name error\n"));
	return nullptr;
}

std::shared_ptr<HazeCompilerInitListValue> HazeCompiler::GetInitializeListValue()
{
	return InitializeListValue;
}

bool HazeCompiler::IsClass(const HAZE_STRING& Name)
{
	auto& Module = GetCurrModule();
	if (Module->FindClass(Name))
	{
		return true;
	}
	else
	{
		for (auto& Iter : Module->Vector_ImportModule)
		{
			if (Iter->FindClass(Name))
			{
				return true;
			}
		}
	}

	return false;
}

const HAZE_CHAR* HazeCompiler::GetClassName(const HAZE_STRING& Name)
{
	for (auto& Iter : HashMap_CompilerModule)
	{
		auto Class = Iter.second->FindClass(Name);
		if (Class)
		{
			return Class->GetName().c_str();
		}
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenConstantValue(const HazeValue& Var)
{
	switch (Var.Type)
	{
	case HazeValueType::Bool:
	{
		auto it = HashMap_BoolConstantValue.find(Var.Value.Bool);
		if (it != HashMap_BoolConstantValue.end())
		{
			return it->second;
		}
		HashMap_BoolConstantValue[Var.Value.Bool] = CreateVariable(Var, HazeDataDesc::Constant);
		return HashMap_BoolConstantValue[Var.Value.Bool];
	}
	case HazeValueType::Int:
	{
		auto it = HashMap_IntConstantValue.find(Var.Value.Int);
		if (it != HashMap_IntConstantValue.end())
		{
			return it->second;
		}
		HashMap_IntConstantValue[Var.Value.Int] = CreateVariable(Var, HazeDataDesc::Constant);
		return HashMap_IntConstantValue[Var.Value.Int];
	}
	case HazeValueType::Long:
	{
		auto it = HashMap_LongConstantValue.find(Var.Value.Long);
		if (it != HashMap_LongConstantValue.end())
		{
			return it->second;
		}
		HashMap_LongConstantValue[Var.Value.Long] = CreateVariable(Var, HazeDataDesc::Constant);
		return HashMap_LongConstantValue[Var.Value.Long];
	}
	case HazeValueType::UnsignedInt:
	{
		auto it = HashMap_UnsignedIntConstantValue.find(Var.Value.UnsignedInt);
		if (it != HashMap_UnsignedIntConstantValue.end())
		{
			return it->second;
		}
		HashMap_UnsignedIntConstantValue[Var.Value.UnsignedInt] = CreateVariable(Var, HazeDataDesc::Constant);
		return HashMap_UnsignedIntConstantValue[Var.Value.UnsignedInt];
	}
	case HazeValueType::UnsignedLong:
	{
		auto it = HashMap_UnsignedLongConstantValue.find(Var.Value.UnsignedLong);
		if (it != HashMap_UnsignedLongConstantValue.end())
		{
			return it->second;
		}
		HashMap_UnsignedLongConstantValue[Var.Value.UnsignedLong] = CreateVariable(Var, HazeDataDesc::Constant);
		return HashMap_UnsignedLongConstantValue[Var.Value.UnsignedLong];
	}
	case HazeValueType::Float:
	{
		auto it = HashMap_FloatConstantValue.find(Var.Value.Float);
		if (it != HashMap_FloatConstantValue.end())
		{
			return it->second;
		}
		HashMap_FloatConstantValue[Var.Value.Float] = CreateVariable(Var, HazeDataDesc::Constant);
		return HashMap_FloatConstantValue[Var.Value.Float];
	}
	case HazeValueType::Double:
	{
		auto it = HashMap_DobuleConstantValue.find(Var.Value.Double);
		if (it != HashMap_DobuleConstantValue.end())
		{
			return it->second;
		}
		HashMap_DobuleConstantValue[Var.Value.Double] = CreateVariable(Var, HazeDataDesc::Constant);
		return HashMap_DobuleConstantValue[Var.Value.Double];
	}
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenStringVariable(HAZE_STRING& String)
{
	return GetCurrModule()->GetGlobalStringVariable(String);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetGlobalVariable(const HAZE_STRING& Name)
{
	return GetCurrModule()->GetGlobalVariable(Name);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetLocalVariable(const HAZE_STRING& Name)
{
	return  GetCurrModule()->GetCurrFunction()->GetLocalVariable(Name);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetConstantValueInt_1()
{
	static HazeValue Value;
	if (Value.Type != HazeValueType::Int || Value.Value.Int != 1)
	{
		Value.Type = HazeValueType::Int; 
		Value.Value.Int = 1;
	}

	return GenConstantValue(Value);
}

void HazeCompiler::SetInsertBlock(std::shared_ptr<HazeBaseBlock> BB)
{
	InsertBaseBlock = BB;
}

void HazeCompiler::ClearBlockPoint()
{
	InsertBaseBlock = nullptr;
}

void HazeCompiler::AddImportModuleToCurrModule(HazeCompilerModule* Module)
{
	/*for (auto& i : HashMap_CompilerModule)
	{
		if (i.second.get() == Module)
		{
			std::wcout << GetCurrModuleName()<< " add module " << i.first << std::endl;
		}
	}*/

	for (auto& Iter : GetCurrModule()->Vector_ImportModule)
	{
		if (Iter == Module)
		{
			return;
		}
	}

	GetCurrModule()->Vector_ImportModule.push_back(Module);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMov(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value)
{
	Alloca->StoreValue(Value);
	GetCurrModule()->GenIRCode_BinaryOperater(Alloca, Value, InstructionOpCode::MOV);

	return Alloca;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineVariable& Variable, std::shared_ptr<HazeCompilerValue> ArraySize)
{
	return Function->CreateLocalVariable(Variable, ArraySize);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& Module, const HazeDefineVariable& Var, std::shared_ptr<HazeCompilerValue> ArraySize)
{
	return Module->CreateGlobalVariable(Var, ArraySize);
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

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateAnd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GetCurrModule()->CreateMod(Left, Right);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateOr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GetCurrModule()->CreateOr(Left, Right);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(Function, Param, ThisPointerTo);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateArrayInit(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> InitList)
{
	return GetCurrModule()->CreateArrayInit(Array, InitList);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateArrayElement(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> Index)
{
	auto ArrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(Array);
	return std::make_shared<HazeCompilerArrayElementValue>(GetCurrModule().get(), HazeDefineType(ArrayValue->GetArrayType().SecondaryType,
		ArrayValue->GetArrayType().CustomName), HazeDataDesc::ArrayElement, 0, Array.get(), Index.get());
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateNew(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineType& Data)
{
	return Function->CreateNew(Data);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateInc(std::shared_ptr<HazeCompilerValue> Value, bool IsPreInc)
{
	return GetCurrModule()->CreateInc(Value, IsPreInc);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateDec(std::shared_ptr<HazeCompilerValue> Value, bool IsPreDec)
{
	return GetCurrModule()->CreateDec(Value, IsPreDec);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateOperatorAssign(HazeOperatorAssign Type, std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GetCurrModule()->CreateOpAssign(Type, Left, Right);
}

void HazeCompiler::CreateJmpFromBlock(std::shared_ptr<HazeBaseBlock> FromBlock, std::shared_ptr<HazeBaseBlock> ToBlock, bool IsJmpL)
{
	GetCurrModule()->GenIRCode_JmpFrom(FromBlock, ToBlock, IsJmpL);
}

void HazeCompiler::CreateJmpToBlock(std::shared_ptr<HazeBaseBlock> Block, bool IsJmpL)
{
	GetCurrModule()->GenIRCode_JmpTo(Block, IsJmpL);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateIntCmp(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	GetCurrModule()->GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::CMP);
	return GetRegister(CMP_REGISTER);
}

void HazeCompiler::CreateCompareJmp(HazeCmpType CmpType, std::shared_ptr<HazeBaseBlock> IfJmpBlock, std::shared_ptr<HazeBaseBlock> ElseJmpBlock, bool IfNullJmpOut, bool ElseNullJmpOut)
{
	GetCurrModule()->GenIRCode_Cmp(CmpType, IfJmpBlock, ElseJmpBlock, IfNullJmpOut, ElseNullJmpOut);
}