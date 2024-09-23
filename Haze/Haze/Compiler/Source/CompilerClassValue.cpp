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
	m_Data = m_OwnerClass->CreateVariableCopyClassMember(compilerModule, scope);
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

Share<CompilerValue> CompilerClassValue::GetMember(const HString& name, HString* nameSpace)
{
	auto index = GetMemberIndex(name, nameSpace);
	if (index < 0)
	{
		COMPILER_ERR_MODULE_W("未能在类<%s>中找到<%s>成员", m_OwnerClass->GetName().c_str(), name.c_str());
	}

	return m_Data.size() > index  ? m_Data[index] : nullptr;
}

int CompilerClassValue::GetMemberIndex(const HString& memberName, HString* nameSpace)
{
	return m_OwnerClass->GetMemberIndex(memberName, nameSpace);
}

int CompilerClassValue::GetMemberIndex(CompilerValue* value)
{
	uint64 index = 0;
	for (uint64 i = 0; i < m_Data.size(); i++)
	{
		if (m_Data[i].get() == value)
		{
			return (int)index;
		}

		index++;
	}

	return -1;
}

bool CompilerClassValue::GetMemberName(const CompilerValue* memberValue, HString& outName, bool getOffset, V_Array<Pair<uint64, CompilerValue*>>* offsets)
{
	//return m_OwnerClass->GetMemberName(this, memberValue, outName, getOffset, offsets);

	/*uint32 count = 0;
	for (size_t i = 0; i < classValue->m_Data.size(); i++)
	{
		for (size_t j = 0; j < classValue->m_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, classValue->m_Data[i].second[j] }, value, outName, getOffset, offsets))
			{
				if (getOffset)
				{
					offsets->push_back({ m_Offsets[count], classValue->m_Data[i].second[j].get() });
				}
				return true;
			}

			count++;
		}
	}*/

	return false;
}