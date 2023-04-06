#include "HazeCompilerHelper.h"

#include <fstream>
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"

HAZE_STRING GetHazeClassFunctionName(const HAZE_STRING& ClassName, const HAZE_STRING& FunctionName)
{
	return ClassName + HAZE_TEXT("@") + FunctionName;
}

void HazeCompilerStream(HAZE_STRING_STREAM& Stream, HazeCompilerValue* Value)
{
	{
		const auto& V = Value->GetValue();
		switch (V.Type)
		{
		case HazeValueType::Bool:
			Stream << V.Value.Bool;
			break;
		case HazeValueType::Char:
			Stream << V.Value.Char;
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
	case HazeValueType::Char:
		OFStream << V.Value.Char;
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

std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* Module, const HazeDefineVariable& Var, HazeDataDesc Scope)
{
	switch (Var.Type.PrimaryType)
	{
	case HazeValueType::Void:
	case HazeValueType::Bool:
	case HazeValueType::Char:
	case HazeValueType::Int:
	case HazeValueType::Float:
	case HazeValueType::Long:
	case HazeValueType::Double:
	case HazeValueType::UnsignedInt:
	case HazeValueType::UnsignedLong:
	case HazeValueType::MultiVariable:
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

std::shared_ptr<HazeCompilerValue> CreateVariable(const HazeValue& Var, HazeDataDesc Scope)
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

HAZE_STRING GetObjectName(const HAZE_STRING& InName)
{
	size_t Pos = InName.find(HAZE_CLASS_POINTER_ATTR);
	if (Pos != HAZE_STRING::npos)
	{
		return InName.substr(0, Pos);
	}
	else
	{
		Pos = InName.find(HAZE_CLASS_ATTR);
		if (Pos != HAZE_STRING::npos)
		{
			return InName.substr(0, Pos);
		}
	}

	return InName;
}

std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* Module, const HAZE_STRING& InName)
{
	bool IsPointer;
	return GetObjectMember(Module, InName, IsPointer);
}

std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* Module, const HAZE_STRING& InName, bool& IsPointer)
{
	HAZE_STRING ObjectName;
	HAZE_STRING MemberName;
	return GetObjectNameAndMemberName(Module, InName, ObjectName, MemberName, IsPointer);
}

std::shared_ptr<HazeCompilerValue> GetObjectNameAndMemberName(HazeCompilerModule* Module, const HAZE_STRING& InName, HAZE_STRING& OutObjectName, HAZE_STRING& OutMemberName, bool& IsPointer)
{
	size_t Pos = InName.find(HAZE_CLASS_POINTER_ATTR);
	if (Pos != HAZE_STRING::npos)
	{
		IsPointer = true;
		OutObjectName = InName.substr(0, Pos);
		OutMemberName = InName.substr(Pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());

		auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Module->GetCurrFunction()->GetLocalVariable(OutObjectName));
		auto ClassValue = static_cast<HazeCompilerClassValue*>(PointerValue->GetPointerValue());
		return ClassValue->GetMember(OutMemberName);
	}
	else
	{
		Pos = InName.find(HAZE_CLASS_ATTR);
		if (Pos != HAZE_STRING::npos)
		{
			IsPointer = false;
			OutObjectName = InName.substr(0, Pos);
			OutMemberName = InName.substr(Pos + HAZE_STRING(HAZE_CLASS_ATTR).size());

			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Module->GetCurrFunction()->GetLocalVariable(OutObjectName));
			return ClassValue->GetMember(OutMemberName);
			
		}
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* Module, const HAZE_STRING& InName)
{
	bool IsPointer;
	return GetObjectFunction(Module, InName, IsPointer);
}

std::shared_ptr<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* Module, const HAZE_STRING& InName, bool& IsPointer)
{
	HAZE_STRING ObjectName;
	HAZE_STRING FunctionName;
	return GetObjectNameAndFunctionName(Module, InName, ObjectName, FunctionName, IsPointer);
}

std::shared_ptr<HazeCompilerFunction> GetObjectNameAndFunctionName(HazeCompilerModule* Module, const HAZE_STRING& InName, HAZE_STRING& OutObjectName, HAZE_STRING& OutFunctionName, bool& IsPointer)
{
	size_t Pos = InName.find(HAZE_CLASS_POINTER_ATTR);
	if (Pos != HAZE_STRING::npos)
	{
		IsPointer = true;
		OutObjectName = InName.substr(0, Pos);
		OutFunctionName = InName.substr(Pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());

		auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Module->GetCurrFunction()->GetLocalVariable(OutObjectName));
		auto ClassValue = static_cast<HazeCompilerClassValue*>(PointerValue->GetPointerValue());
		return ClassValue->GetOwnerClass()->FindFunction(OutFunctionName);
		
	}
	else
	{
		Pos = InName.find(HAZE_CLASS_ATTR);
		if (Pos != HAZE_STRING::npos)
		{
			IsPointer = false;
			OutObjectName = InName.substr(0, Pos);
			OutFunctionName = InName.substr(Pos + HAZE_STRING(HAZE_CLASS_ATTR).size());

			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Module->GetCurrFunction()->GetLocalVariable(OutObjectName));
			return ClassValue->GetOwnerClass()->FindFunction(OutFunctionName);
		}
	}

	return nullptr;
}


