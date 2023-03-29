#include "HazeCompilerHelper.h"

#include <fstream>
#include "HazeCompilerModule.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"

void HazeCompilerStream(HAZE_STRING_STREAM& Stream, HazeCompilerValue* Value)
{
	{
		const auto& V = Value->GetValue();
		switch (V.Type)
		{
		case HazeValueType::Bool:
			Stream << V.Value.Bool;
			break;
		case HazeValueType::Short:
			Stream << V.Value.Short;
			break;
		case HazeValueType::UnsignedShort:
			Stream << V.Value.UnsignedShort;
			break;
		case HazeValueType::Int:
			Stream << V.Value.Int;
			break;
		case HazeValueType::Float:
			Stream << V.Value.Float;
			break;
		case HazeValueType::UnsignedInt:
			Stream << V.Value.UnsignedInt;
			break;
		case HazeValueType::Long:
			Stream << V.Value.Long;
			break;
		case HazeValueType::Double:
			Stream << V.Value.Double;
			break;
		case HazeValueType::UnsignedLong:
			Stream << V.Value.UnsignedLong;
			break;
		default:
			break;
		}
	}
}

void HazeCompilerOFStream(HAZE_OFSTREAM& OFStream, HazeCompilerValue* Value)
{
	const auto& V = Value->GetValue();
	switch (V.Type)
	{
	case HazeValueType::Bool:
		OFStream << V.Value.Bool;
		break;
	case HazeValueType::Short:
		OFStream << V.Value.Short;
		break;
	case HazeValueType::UnsignedShort:
		OFStream << V.Value.UnsignedShort;
		break;
	case HazeValueType::Int:
		OFStream << V.Value.Int;
		break;
	case HazeValueType::Float:
		OFStream << V.Value.Float;
		break;
	case HazeValueType::UnsignedInt:
		OFStream << V.Value.UnsignedInt;
		break;
	case HazeValueType::Long:
		OFStream << V.Value.Long;
		break;
	case HazeValueType::Double:
		OFStream << V.Value.Double;
		break;
	case HazeValueType::UnsignedLong:
		OFStream << V.Value.UnsignedLong;
		break;
	default:
		break;
	}
}

std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* Module, const HazeDefineVariable& Var, InstructionScopeType Scope)
{
	switch (Var.Type.PrimaryType)
	{
	case HazeValueType::Void:
	case HazeValueType::Bool:
	case HazeValueType::Char:
	case HazeValueType::Short:
	case HazeValueType::Int:
	case HazeValueType::Float:
	case HazeValueType::Long:
	case HazeValueType::Double:
	case HazeValueType::UnsignedShort:
	case HazeValueType::UnsignedInt:
	case HazeValueType::UnsignedLong:
		return std::make_shared<HazeCompilerValue>(Module, Var.Type, Scope);
		break;
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
		return std::make_shared<HazeCompilerPointerValue>(Module, Var.Type, Scope);
		break;
	case HazeValueType::Class:
		return std::make_shared<HazeCompilerClassValue>(Module, Var.Type, Scope);
		break;
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> CreateVariable(const HazeValue& Var, InstructionScopeType Scope)
{
	return std::make_shared<HazeCompilerValue>(Var, Scope);
}

void StreamPointerValue(HAZE_STRING_STREAM& HSS, std::shared_ptr<HazeCompilerValue> Value)
{
	auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
	if (Value->GetValue().Type == HazeValueType::PointerBase)
	{
		HSS << " " << (unsigned int)PointerValue->GetPointerType().PrimaryType;
	}
	else
	{
		HSS << " " << PointerValue->GetPointerType().CustomName;
	}
}

void StreamClassValue(HAZE_STRING_STREAM& HSS, std::shared_ptr<HazeCompilerValue> Value)
{
	auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Value);
	HSS << " " << ClassValue->GetOwnerClassName();
}

void StreamCompilerValue(HAZE_STRING_STREAM& HSS, InstructionOpCode InsCode, std::shared_ptr<HazeCompilerValue> Value, const HAZE_CHAR* DefaultName)
{
	HSS << GetInstructionString(InsCode) << " " << (uint)Value->GetValue().Type;
	if (DefaultName)
	{
		HSS << " " << DefaultName;
	}
	HSS << " " << (uint)Value->GetScope();

	if (Value->IsPointer())
	{
		StreamPointerValue(HSS, Value);
	}
	else if (Value->IsClass())
	{
		StreamClassValue(HSS, Value);
	}

	HSS << std::endl;
}

void StreamDefineVariable(HAZE_STRING_STREAM& HSS, const HazeDefineVariable& DefineVariable)
{
	HSS << DefineVariable.Name << " " << HAZE_CAST_VALUE_TYPE(DefineVariable.Type.PrimaryType);
	if (DefineVariable.Type.PrimaryType == HazeValueType::PointerBase)
	{
		HSS << " " << HAZE_CAST_VALUE_TYPE(DefineVariable.Type.PointerToType);
	}
	else if (DefineVariable.Type.PrimaryType == HazeValueType::PointerClass || 
		DefineVariable.Type.PrimaryType == HazeValueType::Class)
	{
		HSS << " " << DefineVariable.Type.CustomName;
	}
	
	HSS << std::endl;
}


