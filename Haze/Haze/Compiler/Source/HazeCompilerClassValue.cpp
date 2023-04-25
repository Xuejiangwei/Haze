#include "HazeCompilerClassValue.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerClass.h"

HazeCompilerClassValue::HazeCompilerClassValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count)
	: HazeCompilerValue(Module, DefineType, Scope, Count)
{
	OwnerClass = Module->FindClass(DefineType.CustomName).get();

	const auto Members = OwnerClass->GetClassMemberData();
	for (size_t i = 0; i < Members.size(); i++)
	{
		Vector_Data.push_back({ Members[i].first,{} });
		for (size_t j = 0; j < Members[i].second.size(); j++)
		{
			Vector_Data.back().second.push_back(CreateVariable(Module, Members[i].second[j], Members[i].first, 0));
		}
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
	auto Index = OwnerClass->GetMemberIndex(Name);
	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			if (Index-- == 0)
			{
				return Vector_Data[i].second[j];
			}
		}
	}

	return nullptr;
}

void HazeCompilerClassValue::GetMemberName(const std::shared_ptr<HazeCompilerValue>& MemberValue, HAZE_STRING& OutName)
{
	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			if (MemberValue == Vector_Data[i].second[j])
			{
				OutName = OwnerClass->GetClassMemberData()[i].second[j].Name;
				return;
			}
		}
	}
}

void HazeCompilerClassValue::GetMemberName(const HazeCompilerValue* MemberValue, HAZE_STRING& OutName)
{
	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			if (MemberValue == Vector_Data[i].second[j].get())
			{
				OutName = OwnerClass->GetClassMemberData()[i].second[j].Name;
				return;
			}
		}
	}
}
