#include "HazeCompilerClassValue.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerClass.h"

//extern std::shared_ptr<HazeCompilerValue> CreateVariableImpl(HazeCompilerModule* Module, const HazeDefineType& Type, HazeVariableScope Scope, HazeDataDesc Desc, int Count,
//	std::shared_ptr<HazeCompilerValue> RefValue, std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize, HazeValue* DefaultValue, std::vector<HazeDefineType>* Vector_Param);

HazeCompilerClassValue::HazeCompilerClassValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count)
	: HazeCompilerValue(compilerModule, defineType, scope, desc, count)
{
	m_OwnerClass = compilerModule->GetClass(defineType.CustomName).get();
	m_Data = std::move(CreateVariableCopyClassMember(compilerModule, scope, m_OwnerClass));
}

HazeCompilerClassValue::~HazeCompilerClassValue()
{
}

unsigned int HazeCompilerClassValue::GetSize()
{
	return m_OwnerClass->GetDataSize();
}

const HAZE_STRING& HazeCompilerClassValue::GetOwnerClassName()
{
	return m_OwnerClass->GetName();
}

std::shared_ptr<HazeCompilerValue> HazeCompilerClassValue::GetMember(const HAZE_STRING& name)
{
	auto index = m_OwnerClass->GetMemberIndex(name);

	for (size_t i = 0; i < m_Data.size(); i++)
	{
		for (size_t j = 0; j < m_Data[i].second.size(); j++)
		{
			if (index == 0)
			{
				return m_Data[i].second[j];
			}
			index--;
		}
	}

	return nullptr;
}

void HazeCompilerClassValue::GetMemberName(const std::shared_ptr<HazeCompilerValue>& memberValue, HAZE_STRING& outName)
{
	m_OwnerClass->GetMemberName(memberValue, outName);
}

void HazeCompilerClassValue::GetMemberName(const HazeCompilerValue* memberValue, HAZE_STRING& outName)
{
	m_OwnerClass->GetMemberName(this, memberValue, outName);
}