#include "HazeCompilerClassValue.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerClass.h"

extern std::shared_ptr<HazeCompilerValue> CreateVariableImpl(HazeCompilerModule* Module, const HazeDefineType& Type, HazeDataDesc Scope, int Count,
	std::shared_ptr<HazeCompilerValue> RefValue, std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize, HazeValue* DefaultValue, std::vector<HazeDefineType>* Vector_Param);

HazeCompilerClassValue::HazeCompilerClassValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count)
	: HazeCompilerValue(Module, DefineType, Scope, Count)
{
	OwnerClass = Module->FindClass(DefineType.CustomName).get();
	Vector_Data = std::move(CreateVariableCopyClassMember(Module, OwnerClass));
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
	auto Index = OwnerClass->GetMemberIndex(Name);

	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			if (Index == 0)
			{
				return Vector_Data[i].second[j];
			}
			Index--;
		}
	}

	return nullptr;
}

void HazeCompilerClassValue::GetMemberName(const std::shared_ptr<HazeCompilerValue>& MemberValue, HAZE_STRING& OutName)
{
	OwnerClass->GetMemberName(MemberValue, OutName);
}

void HazeCompilerClassValue::GetMemberName(const HazeCompilerValue* MemberValue, HAZE_STRING& OutName)
{
	OwnerClass->GetMemberName(MemberValue, OutName);
}

