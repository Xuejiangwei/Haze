#include "HazeCompilerClassValue.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerClass.h"

HazeCompilerClassValue::HazeCompilerClassValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, InstructionScopeType Scope)
	: HazeCompilerValue(Module, DefineType, Scope)
{
	OwnerClass = Module->FindClass(DefineType.CustomName).get();

	const auto Members = OwnerClass->GetClassMemberData();
	for (size_t i = 0; i < Members.size(); i++)
	{
		Vector_Data.push_back(CreateVariable(Module, Members[i], InstructionScopeType::ClassMember_Local));
	}
}

HazeCompilerClassValue::~HazeCompilerClassValue()
{
}

unsigned int HazeCompilerClassValue::GetSize()
{
	return OwnerClass->GetDataSize();
}

const HAZE_STRING& HazeCompilerClassValue::GetOwnerClassName()
{
	return OwnerClass->GetName();
}

std::shared_ptr<HazeCompilerValue> HazeCompilerClassValue::GetMember(const HAZE_STRING& Name)
{
	return Vector_Data[OwnerClass->GetMemberIndex(Name)];
}

void HazeCompilerClassValue::GetMemberName(const std::shared_ptr<HazeCompilerValue>& MemberValue, HAZE_STRING& OutName)
{
	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		if (MemberValue == Vector_Data[i])
		{
			OutName = OwnerClass->GetClassMemberData()[i].Name;
			return;
		}
	}
}
