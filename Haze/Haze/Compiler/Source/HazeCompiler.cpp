#include <filesystem>

#include "HazeCompiler.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerHelper.h"

#include "HazeBaseBlock.h"

static std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> HashMap_GlobalRegister = {
	{ RET_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("Ret_Register")), HAZE_TEXT("")), HazeDataDesc::RegisterRet) },
	{ NEW_REGISTER, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void, HAZE_TEXT("New_Register")), HAZE_TEXT("")), HazeDataDesc::RegisterNew) },
};

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
	HashMap_CompilerModule[GetCurrModuleName()] = std::make_unique<HazeCompilerModule>(ModuleName);
	return true;
}

void HazeCompiler::FinishModule()
{
	GenModuleCodeFile();
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

const HAZE_CHAR* HazeCompiler::GetRegisterName(std::shared_ptr<HazeCompilerValue>& Value)
{
	for (auto& Iter : HashMap_GlobalRegister)
	{
		if (Iter.second == Value)
		{
			return Iter.first;
		}
	}

	return nullptr;
}

bool HazeCompiler::IsClass(const HAZE_STRING& Name)
{
	for (auto& Iter : HashMap_CompilerModule)
	{
		if (Iter.second->FindClass(Name))
		{
			return true;
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

void HazeCompiler::SetInsertBlock(std::shared_ptr<HazeBaseBlock> BB)
{
	InsertBaseBlock = BB;
}

void HazeCompiler::ClearBlockPoint()
{
	InsertBaseBlock = nullptr;
}

void HazeCompiler::StoreValue(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value)
{
	GetCurrModule()->GenIRCode_BinaryOperater(Alloca, Value, InstructionOpCode::MOV);

	Alloca->StoreValue(Value);

	ClearFunctionTemp();
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineVariable& Variable)
{
	return Function->CreateLocalVariable(Variable);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& Module, const HazeDefineVariable& Var)
{
	return Module->CreateGlobalVariable(Var);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateRet(std::shared_ptr<HazeCompilerValue> Value)
{
	GetCurrModule()->GenIRCode_Ret(Value);
	return Value;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GetCurrModule()->CreateAdd(Left, Right);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateSub(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GetCurrModule()->GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::SUB);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMul(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GetCurrModule()->GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::MUL);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateDiv(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GetCurrModule()->GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::DIV);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& Param)
{
	auto& Module = GetCurrModule();
	
	return Module->CreateFunctionCall(Function, Param);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateNew(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineType& Data)
{
	Function->CreateNew(Data);
	return GetRegister(NEW_REGISTER);
}

void HazeCompiler::ClearFunctionTemp()
{
	std::shared_ptr<HazeCompilerFunction> Function = GetCurrModule()->GetCurrFunction();
	if (Function)
	{
		Function->GetTopBaseBlock()->ClearTempIRCode();
	}
}
