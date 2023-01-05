#include "HazeCompiler.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerModule.h"

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

	CurrModule = ModuleName;
	MapModules[CurrModule] = std::make_unique<HazeCompilerModule>(ModuleName);
	return true;
}

void HazeCompiler::FinishModule()
{
	GenModuleCodeFile();
}

void HazeCompiler::GenModuleCodeFile()
{
	MapModules[CurrModule]->GenCodeFile();
}

std::unique_ptr<HazeCompilerModule>& HazeCompiler::GetCurrModule()
{
	return MapModules[CurrModule];
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GenConstantValue(const HazeValue& Var)
{
	switch (Var.Type)
	{
	case HazeValueType::Bool:
	{
		auto it = MapBoolConstantValue.find(Var.Value.Bool);
		if (it != MapBoolConstantValue.end())
		{
			return it->second;
		}
		MapBoolConstantValue[Var.Value.Bool] = std::make_shared<HazeCompilerValue>(Var);
		return MapBoolConstantValue[Var.Value.Bool];
	}
	case HazeValueType::Char:
	case HazeValueType::Byte:
	case HazeValueType::Short:
	case HazeValueType::Int:
	{
		auto it = MapIntConstantValue.find(Var.Value.Int);
		if (it != MapIntConstantValue.end())
		{
			return it->second;
		}
		MapIntConstantValue[Var.Value.Int] = std::make_shared<HazeCompilerValue>(Var);
		return MapIntConstantValue[Var.Value.Int];
	}
	case HazeValueType::Long:
	{
		auto it = MapLongConstantValue.find(Var.Value.Long);
		if (it != MapLongConstantValue.end())
		{
			return it->second;
		}
		MapLongConstantValue[Var.Value.Long] = std::make_shared<HazeCompilerValue>(Var);
		return MapLongConstantValue[Var.Value.Long];
	}
	case HazeValueType::UnsignedByte:
	case HazeValueType::UnsignedInt:
	{
		auto it = MapUnsignedIntConstantValue.find(Var.Value.UnsignedInt);
		if (it != MapUnsignedIntConstantValue.end())
		{
			return it->second;
		}
		MapUnsignedIntConstantValue[Var.Value.UnsignedInt] = std::make_shared<HazeCompilerValue>(Var);
		return MapUnsignedIntConstantValue[Var.Value.UnsignedInt];
	}
	case HazeValueType::UnsignedLong:
	{
		auto it = MapUnsignedLongConstantValue.find(Var.Value.UnsignedLong);
		if (it != MapUnsignedLongConstantValue.end())
		{
			return it->second;
		}
		MapUnsignedLongConstantValue[Var.Value.UnsignedLong] = std::make_shared<HazeCompilerValue>(Var);
		return MapUnsignedLongConstantValue[Var.Value.UnsignedLong];
	}
	case HazeValueType::Float:
	{
		auto it = MapFloatConstantValue.find(Var.Value.Float);
		if (it != MapFloatConstantValue.end())
		{
			return it->second;
		}
		MapFloatConstantValue[Var.Value.Float] = std::make_shared<HazeCompilerValue>(Var);
		return MapFloatConstantValue[Var.Value.Float];
	}
	case HazeValueType::Double:
	{
		auto it = MapDobuleConstantValue.find(Var.Value.Double);
		if (it != MapDobuleConstantValue.end())
		{
			return it->second;
		}
		MapDobuleConstantValue[Var.Value.Double] = std::make_shared<HazeCompilerValue>(Var);
		return MapDobuleConstantValue[Var.Value.Double];
	}
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetGlobalVariable(const HAZE_STRING& Name)
{
	return GetCurrModule()->GetGlobalVariable(Name);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::GetLocalVariable(const HAZE_STRING& Name)
{
	return  GetCurrModule()->GetCurrFunction()->GetLocalVariable(Name);;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineVariable& Variable)
{
	return Function->CreateLocalVariable(Variable);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& Module, const HazeDefineVariable& Var)
{
	return Module->CreateGlobalVariable(Var);
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	std::shared_ptr<HazeCompilerValue> CompilerValue = std::make_shared<HazeCompilerValue>();
	HazeValue& Value = CompilerValue->GetValue();
	switch (Left->GetValue().Type)
	{
	case HazeValueType::Char:
	case HazeValueType::Byte:
	case HazeValueType::Short:
	case HazeValueType::Int:
	{
		Value.Value.Int = Left->GetValue().Value.Int + Right->GetValue().Value.Int;
		GetCurrModule()->GenASS_Add(Left, Right);
	}
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
	}

	Value.Type = Left->GetValue().Type;
	return CompilerValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompiler::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param)
{
	auto& Module = GetCurrModule();
	
	return Module->CreateFunctionCall(Function, Param);
}
