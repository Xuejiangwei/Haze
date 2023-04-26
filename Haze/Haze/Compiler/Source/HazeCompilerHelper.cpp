#include "HazeCompilerHelper.h"

#include <fstream>
#include "HazeLog.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"

HAZE_STRING GetHazeClassFunctionName(const HAZE_STRING& ClassName, const HAZE_STRING& FunctionName)
{
	return ClassName + HAZE_TEXT("@") + FunctionName;
}

HAZE_STRING GetLocalVariableName(const HAZE_STRING& Name, std::shared_ptr<HazeCompilerValue> Value)
{
	static HAZE_STRING_STREAM HSS;

	HSS.str(HAZE_TEXT(""));
	HSS << Name;
	if (Value->GetCount() > 0)
	{
		HSS << HAZE_LOCAL_VARIABLE_CONBINE << Value->GetCount();
	}

	return HSS.str();
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

void HazeCompilerOFStream(HAZE_OFSTREAM& OFStream, std::shared_ptr<HazeCompilerValue> Value, bool StreamValue)
{
	const auto& V = Value->GetValue();
	
	if (StreamValue)
	{
		switch (V.Type)
		{
		case HazeValueType::Bool:
			OFStream << " " << V.Value.Bool;
			break;
		case HazeValueType::Char:
			OFStream << " " << V.Value.Char;
			break;
		case HazeValueType::Int:
			OFStream << " " << V.Value.Int;
			break;
		case HazeValueType::Float:
			OFStream << " " << V.Value.Float;
			break;
		case HazeValueType::UnsignedInt:
			OFStream << V.Value.UnsignedInt;
			break;
		case HazeValueType::Long:
			OFStream << " " << V.Value.Long;
			break;
		case HazeValueType::Double:
			OFStream << " " << V.Value.Double;
			break;
		case HazeValueType::UnsignedLong:
			OFStream << " " << V.Value.UnsignedLong;
			break;
		default:
			break;
		}
	}
	else
	{
		OFStream << " " << HAZE_CAST_VALUE_TYPE(V.Type);
		if (Value->IsPointer())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
			if (PointerValue->IsPointerBase())
			{
				OFStream << " " << HAZE_CAST_VALUE_TYPE(PointerValue->GetPointerType().PrimaryType);
			}
			else if (PointerValue->IsPointerClass())
			{
				OFStream << " " << PointerValue->GetPointerType().CustomName;
			}
		}
		else if (Value->IsClass())
		{
			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Value);
			OFStream << " " << ClassValue->GetOwnerClassName();
		}
		else if (V.Type == HazeValueType::MultiVariable)
		{
			HAZE_TO_DO(HazeCompilerOFStream Function MultiVariable);
		}
	}
}

std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* Module, const HazeDefineVariable& Var, HazeDataDesc Scope, int Count, std::shared_ptr<HazeCompilerValue> ArraySize)
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
		return std::make_shared<HazeCompilerValue>(Module, Var.Type, Scope, Count);
	case HazeValueType::Array:
		return std::make_shared<HazeCompilerArrayValue>(Module, Var.Type, Scope, Count, ArraySize.get());
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
		return std::make_shared<HazeCompilerPointerValue>(Module, Var.Type, Scope, Count);
	case HazeValueType::Class:
		return std::make_shared<HazeCompilerClassValue>(Module, Var.Type, Scope, Count);
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
	HSS << GetInstructionString(InsCode) << " " << (uint32)Value->GetValue().Type;
	if (DefaultName)
	{
		HSS << " " << DefaultName;
	}
	HSS << " " << (uint32)Value->GetScope();

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
		HSS << " " << HAZE_CAST_VALUE_TYPE(DefineVariable.Type.SecondaryType);
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
		auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(PointerValue->GetPointerValue()->GetShared());
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
		auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(PointerValue->GetPointerValue()->GetShared());
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

bool TrtGetVariableName(HazeCompilerFunction* Function, const std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>& Data, const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	if (Data.second == Value)
	{
		OutName = GetLocalVariableName(Data.first, Data.second);
		return true;
	}
	if (Value->IsClassMember())
	{
		if (Data.second->IsPointerClass())
		{
			auto Pointer = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Data.second);
			if (Function)
			{
				if ((void*)Pointer->GetPointerValue()!= Function->GetClass())
				{
					auto Class = dynamic_cast<HazeCompilerClassValue*>(Pointer->GetPointerValue());
					Class->GetMemberName(Value, OutName);
					if (!OutName.empty())
					{
						OutName = GetLocalVariableName(Data.first, Data.second) + HAZE_CLASS_POINTER_ATTR + OutName;
						return true;
					}
				}
			}
		}
		else
		{
			auto Class = std::dynamic_pointer_cast<HazeCompilerClassValue>(Data.second);
			Class->GetMemberName(Value, OutName);
			if (!OutName.empty())
			{
				OutName = GetLocalVariableName(Data.first, Data.second) + HAZE_CLASS_ATTR + OutName;
				if (!Value->IsClassPublicMember())
				{
					HAZE_LOG_ERR(HAZE_TEXT("can not access non public member data %s\n"), OutName.c_str());
				}
				return true;
			}
		}
	}
	else if (Value->IsCalssThis())
	{
		if (Data.second->GetValue().Type == HazeValueType::Class)
		{
			auto PointerThis = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
			if (PointerThis->GetPointerValue() == Data.second.get())
			{
				OutName = GetLocalVariableName(Data.first, Data.second);
				return true;
			}
		}
	}
	else if (Value->IsArrayElement())
	{
		if (Data.second->IsArray())
		{
			auto ArrayElement = std::dynamic_pointer_cast<HazeCompilerArrayElementValue>(Value);
			if (ArrayElement->GetArray() == Data.second.get())
			{
				OutName = GetLocalVariableName(Data.first, Data.second);
				return true;
			}
		}
	}

	return false;
}

bool TrtGetVariableName(HazeCompilerFunction* Function, const std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>& Data, const HazeCompilerValue* Value, HAZE_STRING& OutName)
{
	if (Data.second.get() == Value)
	{
		OutName = GetLocalVariableName(Data.first, Data.second);
		return true;
	}
	if (Value->IsClassMember())
	{
		if (Data.second->IsPointerClass())
		{
			auto Pointer = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Data.second);
			if (Function)
			{
				if ((void*)Pointer->GetPointerValue() != Function->GetClass())
				{
					auto Class = dynamic_cast<HazeCompilerClassValue*>(Pointer->GetPointerValue());
					Class->GetMemberName(Value, OutName);
					if (!OutName.empty())
					{
						OutName = GetLocalVariableName(Data.first, Data.second) + HAZE_CLASS_POINTER_ATTR + OutName;
						return true;
					}
				}
			}
		}
		else
		{
			auto Class = std::dynamic_pointer_cast<HazeCompilerClassValue>(Data.second);
			Class->GetMemberName(Value, OutName);
			if (!OutName.empty())
			{
				OutName = GetLocalVariableName(Data.first, Data.second) + HAZE_CLASS_ATTR + OutName;
				if (!Value->IsClassPublicMember())
				{
					HAZE_LOG_ERR(HAZE_TEXT("can not access non public member data %s\n"), OutName.c_str());
				}
				return true;
			}
		}
	}
	else if (Value->IsCalssThis())
	{
		if (Data.second->GetValue().Type == HazeValueType::Class)
		{
			auto PointerThis = static_cast<const HazeCompilerPointerValue*>(Value);
			if (PointerThis->GetPointerValue() == Data.second.get())
			{
				OutName = GetLocalVariableName(Data.first, Data.second);
				return true;
			}
		}
	}
	else if (Value->IsArrayElement())
	{
		if (Data.second->IsArray())
		{
			auto ArrayElement = static_cast<const HazeCompilerArrayElementValue*>(Value);
			if (ArrayElement->GetArray() == Data.second.get())
			{
				OutName = GetLocalVariableName(Data.first, Data.second);
				return true;
			}
		}
	}

	return false;
}


