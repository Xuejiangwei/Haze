#include <filesystem>

#include "HazeCompiler.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerModule.h"

#include "HazeBaseBlock.h"

//static std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> Map_GlobalRegister = {
//	{ ADD_REGISTER, std::make_shared<HazeCompilerValue>(nullptr, HazeDefineData({{HazeToken::Int, HAZE_TEXT("Add_Register")}, HazeCompilerValue::ValueSection::Register} },
//	{ SUB_REGISTER, std::make_shared<HazeCompilerValue>(nullptr, HazeDefineData(HazeToken::Int, HAZE_TEXT("Sub_Register")), HazeCompilerValue::ValueSection::Register) },
//	{ MUL_REGISTER, std::make_shared<HazeCompilerValue>(nullptr, HazeDefineData(HazeToken::Int, HAZE_TEXT("Mul_Register")), HazeCompilerValue::ValueSection::Register) },
//	{ DIV_REGISTER, std::make_shared<HazeCompilerValue>(nullptr, HazeDefineData(HazeToken::Int, HAZE_TEXT("Div_Register")), HazeCompilerValue::ValueSection::Register) },
//	{ RET_REGISTER, std::make_shared<HazeCompilerValue>(nullptr, HazeDefineData(HazeToken::Int, HAZE_TEXT("Ret_Register")), HazeCompilerValue::ValueSection::Register) },
//};

static std::shared_ptr<HazeCompilerValue> RetRegister = std::make_shared<HazeCompilerValue>(nullptr, HazeDefineData(HazeValueType::Null, HAZE_TEXT("Ret_Register")), InstructionScopeType::Register);

HazeCompiler::HazeCompiler()
{
}

HazeCompiler::~HazeCompiler()
{
}

bool HazeCompiler::InitializeCompiler(const HAZE_STRING& ModuleName)
{
	//Éú³ÉÄ£¿é
	auto It = MapModules.find(ModuleName);
	if (It != MapModules.end())
	{
		return false;
	}

	Vector_ModuleNameStack.push_back(ModuleName);
	MapModules[GetCurrModuleName()] = std::make_unique<HazeCompilerModule>(ModuleName);
	return true;
}

void HazeCompiler::FinishModule()
{
	GenModuleCodeFile();
}

void HazeCompiler::GenModuleCodeFile()
{
	MapModules[GetCurrModuleName()]->GenCodeFile();
}

std::unique_ptr<HazeCompilerModule>& HazeCompiler::GetCurrModule()
{
	return MapModules[GetCurrModuleName()];
}

bool HazeCompiler::CurrModuleIsStdLib()
{
	return GetCurrModule()->IsStandardLibrary();
}

std::shared_ptr<HazeCompilerFunction> HazeCompiler::GetFunction(const HAZE_STRING& Name)
{
	std::shared_ptr<HazeCompilerFunction> Function = nullptr;
	for (auto& Iter : MapModules)
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
std::shared_ptr<HazeCompilerValue> HazeCompiler::GetReturnRegister()
{
	return RetRegister;
}

const HAZE_CHAR* HazeCompiler::GetReturnRegisterName()
{
	return HAZE_TEXT("Ret_Register");
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
		HashMap_BoolConstantValue[Var.Value.Bool] = std::make_shared<HazeCompilerValue>(Var, InstructionScopeType::Constant);
		return HashMap_BoolConstantValue[Var.Value.Bool];
	}
	case HazeValueType::Short:
	case HazeValueType::Int:
	{
		auto it = HashMap_IntConstantValue.find(Var.Value.Int);
		if (it != HashMap_IntConstantValue.end())
		{
			return it->second;
		}
		HashMap_IntConstantValue[Var.Value.Int] = std::make_shared<HazeCompilerValue>(Var, InstructionScopeType::Constant);
		return HashMap_IntConstantValue[Var.Value.Int];
	}
	case HazeValueType::Long:
	{
		auto it = HashMap_LongConstantValue.find(Var.Value.Long);
		if (it != HashMap_LongConstantValue.end())
		{
			return it->second;
		}
		HashMap_LongConstantValue[Var.Value.Long] = std::make_shared<HazeCompilerValue>(Var, InstructionScopeType::Constant);
		return HashMap_LongConstantValue[Var.Value.Long];
	}
	case HazeValueType::UnsignedInt:
	{
		auto it = HashMap_UnsignedIntConstantValue.find(Var.Value.UnsignedInt);
		if (it != HashMap_UnsignedIntConstantValue.end())
		{
			return it->second;
		}
		HashMap_UnsignedIntConstantValue[Var.Value.UnsignedInt] = std::make_shared<HazeCompilerValue>(Var, InstructionScopeType::Constant);
		return HashMap_UnsignedIntConstantValue[Var.Value.UnsignedInt];
	}
	case HazeValueType::UnsignedLong:
	{
		auto it = HashMap_UnsignedLongConstantValue.find(Var.Value.UnsignedLong);
		if (it != HashMap_UnsignedLongConstantValue.end())
		{
			return it->second;
		}
		HashMap_UnsignedLongConstantValue[Var.Value.UnsignedLong] = std::make_shared<HazeCompilerValue>(Var, InstructionScopeType::Constant);
		return HashMap_UnsignedLongConstantValue[Var.Value.UnsignedLong];
	}
	case HazeValueType::Float:
	{
		auto it = HashMap_FloatConstantValue.find(Var.Value.Float);
		if (it != HashMap_FloatConstantValue.end())
		{
			return it->second;
		}
		HashMap_FloatConstantValue[Var.Value.Float] = std::make_shared<HazeCompilerValue>(Var, InstructionScopeType::Constant);
		return HashMap_FloatConstantValue[Var.Value.Float];
	}
	case HazeValueType::Double:
	{
		auto it = HashMap_DobuleConstantValue.find(Var.Value.Double);
		if (it != HashMap_DobuleConstantValue.end())
		{
			return it->second;
		}
		HashMap_DobuleConstantValue[Var.Value.Double] = std::make_shared<HazeCompilerValue>(Var, InstructionScopeType::Constant);
		return HashMap_DobuleConstantValue[Var.Value.Double];
	}
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenString(HAZE_STRING& String)
{
	return GetCurrModule()->GetGlobalStringVariable(String);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetGlobalVariable(const HAZE_STRING& Name)
{
	return GetCurrModule()->GetGlobalVariable(Name);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetLocalVariable(const HAZE_STRING& Name)
{
	return  GetCurrModule()->GetCurrFunction()->GetLocalVariable(Name);;
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
	/*std::shared_ptr<HazeCompilerValue> AddR = Map_GlobalRegister[ADD_REGISTER];
	HazeValue& Value = AddR->GetValue();
	Value.Type = Left->GetValue().Type;*/

	/*switch (Left->GetValue().Type)
	{
	case HazeValueType::Char:
	case HazeValueType::Byte:
	case HazeValueType::Short:
	case HazeValueType::Int:
		Value.Value.Int = Left->GetValue().Value.Int + Right->GetValue().Value.Int;
		break;
	case HazeValueType::Long:
		Value.Value.Long = Left->GetValue().Value.Long + Right->GetValue().Value.Long;
		break;
	case HazeValueType::Float:
		Value.Value.Float = Left->GetValue().Value.Float + Right->GetValue().Value.Float;
		break;
	case HazeValueType::Double:
		Value.Value.Double = Left->GetValue().Value.Double + Right->GetValue().Value.Double;
		break;
	case HazeValueType::UnsignedByte:
	case HazeValueType::UnsignedShort:
	case HazeValueType::UnsignedInt:
		Value.Value.UnsignedInt = Left->GetValue().Value.UnsignedInt + Right->GetValue().Value.UnsignedInt;
		break;
	case HazeValueType::UnsignedLong:
		Value.Value.UnsignedLong = Left->GetValue().Value.UnsignedLong + Right->GetValue().Value.UnsignedLong;
		break;
	default:
		break;
	}*/

	return GetCurrModule()->CreateAdd(Left, Right);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateSub(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	/*std::shared_ptr<HazeCompilerValue> SubR = Map_GlobalRegister[SUB_RGISTER];
	HazeValue& Value = SubR->GetValue();
	Value.Type = Left->GetValue().Type;*/

	/*switch (Left->GetValue().Type)
	{
	case HazeValueType::Char:
	case HazeValueType::Byte:
	case HazeValueType::Short:
	case HazeValueType::Int:
		Value.Value.Int = Left->GetValue().Value.Int - Right->GetValue().Value.Int;
		break;
	case HazeValueType::Long:
		Value.Value.Long = Left->GetValue().Value.Long - Right->GetValue().Value.Long;
		break;
	case HazeValueType::Float:
		Value.Value.Float = Left->GetValue().Value.Float - Right->GetValue().Value.Float;
		break;
	case HazeValueType::Double:
		Value.Value.Double = Left->GetValue().Value.Double - Right->GetValue().Value.Double;
		break;
	case HazeValueType::UnsignedByte:
	case HazeValueType::UnsignedShort:
	case HazeValueType::UnsignedInt:
		Value.Value.UnsignedInt = Left->GetValue().Value.UnsignedInt - Right->GetValue().Value.UnsignedInt;
		break;
	case HazeValueType::UnsignedLong:
		Value.Value.UnsignedLong = Left->GetValue().Value.UnsignedLong - Right->GetValue().Value.UnsignedLong;
		break;
	default:
		break;
	}*/

	return GetCurrModule()->GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::SUB);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateMul(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	/*std::shared_ptr<HazeCompilerValue> MulR = Map_GlobalRegister[MUL_REGISTER];
	HazeValue& Value = MulR->GetValue();
	Value.Type = Left->GetValue().Type;*/

	/*switch (Left->GetValue().Type)
	{
	case HazeValueType::Char:
	case HazeValueType::Byte:
	case HazeValueType::Short:
	case HazeValueType::Int:
		Value.Value.Int = Left->GetValue().Value.Int * Right->GetValue().Value.Int;
		break;
	case HazeValueType::Long:
		Value.Value.Long = Left->GetValue().Value.Long * Right->GetValue().Value.Long;
		break;
	case HazeValueType::Float:
		Value.Value.Float = Left->GetValue().Value.Float * Right->GetValue().Value.Float;
		break;
	case HazeValueType::Double:
		Value.Value.Double = Left->GetValue().Value.Double * Right->GetValue().Value.Double;
		break;
	case HazeValueType::UnsignedByte:
	case HazeValueType::UnsignedShort:
	case HazeValueType::UnsignedInt:
		Value.Value.UnsignedInt = Left->GetValue().Value.UnsignedInt * Right->GetValue().Value.UnsignedInt;
		break;
	case HazeValueType::UnsignedLong:
		Value.Value.UnsignedLong = Left->GetValue().Value.UnsignedLong * Right->GetValue().Value.UnsignedLong;
		break;
	default:
		break;
	}*/

	return GetCurrModule()->GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::MUL);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateDiv(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	/*std::shared_ptr<HazeCompilerValue> DivR = Map_GlobalRegister[DIV_REGISTER];
	HazeValue& Value = DivR->GetValue();
	Value.Type = Left->GetValue().Type;*/

	/*switch (Left->GetValue().Type)
	{
	case HazeValueType::Char:
	case HazeValueType::Byte:
	case HazeValueType::Short:
	case HazeValueType::Int:
		Value.Value.Int = Left->GetValue().Value.Int / Right->GetValue().Value.Int;
		break;
	case HazeValueType::Long:
		Value.Value.Long = Left->GetValue().Value.Long / Right->GetValue().Value.Long;
		break;
	case HazeValueType::Float:
		Value.Value.Float = Left->GetValue().Value.Float / Right->GetValue().Value.Float;
		break;
	case HazeValueType::Double:
		Value.Value.Double = Left->GetValue().Value.Double / Right->GetValue().Value.Double;
		break;
	case HazeValueType::UnsignedByte:
	case HazeValueType::UnsignedShort:
	case HazeValueType::UnsignedInt:
		Value.Value.UnsignedInt = Left->GetValue().Value.UnsignedInt / Right->GetValue().Value.UnsignedInt;
		break;
	case HazeValueType::UnsignedLong:
		Value.Value.UnsignedLong = Left->GetValue().Value.UnsignedLong / Right->GetValue().Value.UnsignedLong;
		break;
	default:
		break;
	}*/

	return GetCurrModule()->GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::DIV);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& Param)
{
	auto& Module = GetCurrModule();
	
	return Module->CreateFunctionCall(Function, Param);
}

void HazeCompiler::ClearFunctionTemp()
{
	std::shared_ptr<HazeCompilerFunction> Function = GetCurrModule()->GetCurrFunction();
	if (Function)
	{
		Function->GetTopBaseBlock()->ClearTempIRCode();
	}
}