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

std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* Module, const HazeDefineVariable& Var, InstructionScopeType Scope, std::shared_ptr<HazeCompilerValue> Parent)
{
	switch (Var.Type.Type)
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
		return std::make_shared<HazeCompilerValue>(Module, Var.Type, Scope, Parent);
		break;
	case HazeValueType::Pointer:
		return std::make_shared<HazeCompilerPointerValue>(Module, Var.Type, Scope, Parent);
		break;
	case HazeValueType::Class:
		return std::make_shared<HazeCompilerClassValue>(Module, Var.Type, Scope, Parent);
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
