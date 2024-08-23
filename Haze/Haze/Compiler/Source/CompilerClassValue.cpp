#include "HazePch.h"
#include "CompilerClassValue.h"
#include "CompilerModule.h"
#include "CompilerHelper.h"
#include "CompilerClass.h"

//extern Share<HazeCompilerValue> CreateVariableImpl(HazeCompilerModule* Module, const HazeDefineType& Type, HazeVariableScope Scope, HazeDataDesc Desc, int Count,
//	Share<HazeCompilerValue> RefValue, V_Array<Share<HazeCompilerValue>> ArraySize, HazeValue* DefaultValue, V_Array<HazeDefineType>* Vector_Param);

CompilerClassValue::CompilerClassValue(CompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count)
	: CompilerValue(compilerModule, defineType, scope, desc, count)
{
	m_OwnerClass = compilerModule->GetClass(*defineType.CustomName).get();
	m_Data = Move(CreateVariableCopyClassMember(compilerModule, scope, m_OwnerClass));
}

CompilerClassValue::~CompilerClassValue()
{
}

unsigned int CompilerClassValue::GetSize()
{
	return m_OwnerClass->GetDataSize();
}

const HString& CompilerClassValue::GetOwnerClassName()
{
	return m_OwnerClass->GetName();
}

Share<CompilerValue> CompilerClassValue::GetMember(const HString& name)
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

bool CompilerClassValue::GetMemberName(const CompilerValue* memberValue, HString& outName, bool getOffset, V_Array<uint64>* offsets)
{
	return m_OwnerClass->GetMemberName(this, memberValue, outName, getOffset, offsets);
}