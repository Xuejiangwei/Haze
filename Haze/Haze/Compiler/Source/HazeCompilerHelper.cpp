#include "HazeCompilerHelper.h"

#include "HazeLog.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerPointerArray.h"
#include "HazeCompilerPointerFunction.h"
#include "HazeCompilerRefValue.h"
#include "HazeCompilerClassValue.h"

HAZE_STRING GetHazeClassFunctionName(const HAZE_STRING& ClassName, const HAZE_STRING& FunctionName)
{
	return ClassName + HAZE_CLASS_FUNCTION_CONBINE + FunctionName;
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

void HazeCompilerStream(HAZE_STRING_STREAM& Stream, HazeCompilerValue* Value, bool StreamValue)
{
	if (StreamValue)
	{
		const auto& V = Value->GetValue();
		switch (Value->GetValueType().PrimaryType)
		{
		case HazeValueType::Bool:
			Stream << V.Value.Bool;
			break;
		case HazeValueType::Byte:
			Stream << V.Value.Byte;
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
	else
	{
		Stream << " ";
		Value->GetValueType().StringStreamTo(Stream);
	}
}

void HazeCompilerStream(HAZE_STRING_STREAM& Stream, std::shared_ptr<HazeCompilerValue> Value, bool StreamValue)
{
	HazeCompilerStream(Stream, Value.get(), StreamValue);
}

std::shared_ptr<HazeCompilerValue> CreateVariableImpl(HazeCompilerModule* Module, const HazeDefineType& Type, HazeVariableScope Scope, HazeDataDesc Desc, int Count,
	std::shared_ptr<HazeCompilerValue> AssignValue, std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize, std::vector<HazeDefineType>* Vector_Param)
{
	switch (Type.PrimaryType)
	{
	case HazeValueType::Void:
	case HazeValueType::Bool:
	case HazeValueType::Byte:
	case HazeValueType::Char:
	case HazeValueType::Int:
	case HazeValueType::Float:
	case HazeValueType::Long:
	case HazeValueType::Double:
	case HazeValueType::UnsignedInt:
	case HazeValueType::UnsignedLong:
		return std::make_shared<HazeCompilerValue>(Module, Type, Scope, Desc, Count, AssignValue);
	case HazeValueType::MultiVariable:
	{
		return std::make_shared<HazeCompilerValue>(Module, Type, Scope, Desc, Count, AssignValue);
	}
	case HazeValueType::Array:
		return std::make_shared<HazeCompilerArrayValue>(Module, Type, Scope, Desc, Count, ArraySize);
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
	case HazeValueType::PointerPointer:
		return std::make_shared<HazeCompilerPointerValue>(Module, Type, Scope, Desc, Count);
	case HazeValueType::PointerFunction:
		return std::make_shared<HazeCompilerPointerFunction>(Module, Type, Scope, Desc, Count, Vector_Param ? *Vector_Param : std::vector<HazeDefineType>{});
	case HazeValueType::PointerArray:
		return std::make_shared<HazeCompilerPointerArray>(Module, Type, Scope, Desc, Count, ArraySize);
	case HazeValueType::ReferenceBase:
	case HazeValueType::ReferenceClass:
		if (AssignValue)
		{
			return std::make_shared<HazeCompilerRefValue>(Module, Type, Scope, Desc, Count, AssignValue);
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("创建引用变量失败，必须赋予初始化值!\n"));
			return nullptr;
		}
	case HazeValueType::Class:
		return std::make_shared<HazeCompilerClassValue>(Module, Type, Scope, Desc, Count);
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* Module, const HazeDefineVariable& Var, HazeVariableScope Scope, HazeDataDesc Desc, int Count,
	std::shared_ptr<HazeCompilerValue> RefValue, std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize, std::vector<HazeDefineType>* Vector_Param)
{
	return CreateVariableImpl(Module, Var.Type, Scope, Desc, Count, RefValue, ArraySize, Vector_Param);
}

std::vector<std::pair<HazeDataDesc, std::vector<std::shared_ptr<HazeCompilerValue>>>> CreateVariableCopyClassMember(HazeCompilerModule* Module, HazeVariableScope Scope, HazeCompilerClass* Class)
{
	std::vector<std::pair<HazeDataDesc, std::vector<std::shared_ptr<HazeCompilerValue>>>> Vector_Member;
	for (auto& It : Class->GetClassMemberData())
	{
		Vector_Member.push_back({ It.first, {} });
		Vector_Member.back().second.resize(It.second.size());
		for (size_t i = 0; i < It.second.size(); i++)
		{
			auto& Var = It.second[i].second;
			Vector_Member.back().second[i] = CreateVariableImpl(Module, Var->GetValueType(), Scope, Var->GetVariableDesc(), 0,
				Var->IsRef() ? std::dynamic_pointer_cast<HazeCompilerRefValue>(Var)->GetRefValue() : Var,
				Var->IsArray() ? std::dynamic_pointer_cast<HazeCompilerArrayValue>(Var)->GetArraySize() : Var->IsPointerArray() ?
				std::dynamic_pointer_cast<HazeCompilerPointerArray>(Var)->GetArraySize() : std::vector<std::shared_ptr<HazeCompilerValue>>{},
				Var->IsPointerFunction() ? &const_cast<std::vector<HazeDefineType>&>(std::dynamic_pointer_cast<HazeCompilerPointerFunction>(Var)->GetParamType()) : nullptr);
		}
	}

	return Vector_Member;
}

//std::shared_ptr<HazeCompilerValue> CreateVariable(const HazeValue& Var, HazeDataDesc Scope)
//{
//	return std::make_shared<HazeCompilerValue>(Var, Scope);
//}

void StreamPointerValue(HAZE_STRING_STREAM& HSS, std::shared_ptr<HazeCompilerValue> Value)
{
	auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
	if (Value->GetValueType().PrimaryType == HazeValueType::PointerBase)
	{
		HSS << " " << (uint32)PointerValue->GetValueType().PrimaryType;
	}
	else
	{
		HSS << " " << PointerValue->GetValueType().CustomName;
	}
}

void StreamClassValue(HAZE_STRING_STREAM& HSS, std::shared_ptr<HazeCompilerValue> Value)
{
	auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Value);
	HSS << " " << ClassValue->GetOwnerClassName();
}

void StreamCompilerValue(HAZE_STRING_STREAM& HSS, InstructionOpCode InsCode, std::shared_ptr<HazeCompilerValue> Value, const HAZE_CHAR* DefaultName)
{
	HSS << GetInstructionString(InsCode) << " " << (uint32)Value->GetValueType().PrimaryType;
	if (DefaultName)
	{
		HSS << " " << DefaultName;
	}
	HSS << " " << (uint32)Value->GetVariableDesc();

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
	auto Pos = InName.find(HAZE_CLASS_POINTER_ATTR);
	if (Pos != HAZE_STRING::npos)
	{
		IsPointer = true;
		OutObjectName = InName.substr(0, Pos);
		OutMemberName = InName.substr(Pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());

		auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Module->GetCurrFunction()->GetLocalVariable(OutObjectName));
		auto Class = Module->GetClass(PointerValue->GetValueType().CustomName);

		std::shared_ptr<HazeCompilerClassValue> ClassValue = nullptr;
		if (OutObjectName == HAZE_CLASS_THIS)
		{
			ClassValue = Class->GetThisPointerToValue();
		}
		else
		{
			ClassValue = Class->GetNewPointerToValue();
		}

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
			if (ClassValue)
			{
				return ClassValue->GetMember(OutMemberName);
			}
			else
			{
				ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Module->GetGlobalVariable(OutObjectName));
				if (ClassValue)
				{
					return ClassValue->GetMember(OutMemberName);
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("函数<%s>中未能找到类对象<%s>!\n"), Module->GetCurrFunction()->GetName().c_str(), OutObjectName.c_str());
				}
			}
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
	auto Pos = InName.find(HAZE_CLASS_POINTER_ATTR);
	if (Pos != HAZE_STRING::npos)
	{
		IsPointer = true;
		OutObjectName = InName.substr(0, Pos);
		OutFunctionName = InName.substr(Pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());

		auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Module->GetCurrFunction()->GetLocalVariable(OutObjectName));
		auto Class = Module->GetClass(PointerValue->GetValueType().CustomName);
		return Class->FindFunction(OutFunctionName);
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
			if (ClassValue)
			{
				return ClassValue->GetOwnerClass()->FindFunction(OutFunctionName);
			}
		}
	}

	return nullptr;
}

bool TrtGetVariableName(HazeCompilerFunction* Function, const std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>& Data, const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	return TrtGetVariableName(Function, Data, Value.get(), OutName);
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
			if (Function)
			{
				auto Class = Function->GetModule()->GetClass(Data.second->GetValueType().CustomName);
				Class->GetMemberName(Value, OutName);
				if (!OutName.empty())
				{
					OutName = GetLocalVariableName(Data.first, Data.second) + HAZE_CLASS_POINTER_ATTR + OutName;
					return true;
				}
			}
		}
		else if (Data.second->IsClass())
		{
			auto Class = std::dynamic_pointer_cast<HazeCompilerClassValue>(Data.second);
			Class->GetMemberName(Value, OutName);
			if (!OutName.empty())
			{
				OutName = GetLocalVariableName(Data.first, Data.second) + HAZE_CLASS_ATTR + OutName;
				if (!Value->IsClassPublicMember())
				{
					HAZE_LOG_ERR(HAZE_TEXT("不能够访问类<%s>非公开成员变量<%s>!\n"), Class->GetOwnerClassName().c_str(), OutName.c_str());
				}
				return true;
			}
		}
	}
	else if (Value->IsArrayElement())
	{
		auto ArrayElement = static_cast<const HazeCompilerArrayElementValue*>(Value);
		if (Data.second->IsArray() || Data.second->IsPointer())
		{
			if (ArrayElement->GetArray() == Data.second.get())
			{
				OutName = GetLocalVariableName(Data.first, Data.second);
				return true;
			}
		}
	}

	return false;
}

std::shared_ptr<HazeCompilerValue> GetArrayElementToValue(HazeCompilerModule* Module, std::shared_ptr<HazeCompilerValue> ElementValue, std::shared_ptr<HazeCompilerValue> MovToValue)
{
	auto Compiler = Module->GetCompiler();
	auto ArrayPointer = Compiler->CreatePointerToArrayElement(ElementValue);
	
	return Compiler->CreateMovPV(MovToValue ? MovToValue : Compiler->GetTempRegister(), ArrayPointer);
}
