#include "HazePch.h"
#include "CompilerClassValue.h"
#include "Compiler.h"
#include "CompilerSymbol.h"
#include "CompilerModule.h"
#include "CompilerHelper.h"
#include "CompilerClass.h"

//extern Share<HazeCompilerValue> CreateVariableImpl(HazeCompilerModule* Module, const HazeDefineType& Type, HazeVariableScope Scope, HazeDataDesc Desc, int Count,
//	Share<HazeCompilerValue> RefValue, V_Array<Share<HazeCompilerValue>> ArraySize, HazeValue* DefaultValue, V_Array<HazeDefineType>* Vector_Param);

CompilerClassValue::CompilerClassValue(CompilerModule* compilerModule, const HazeVariableType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count)
	: CompilerValue(compilerModule, defineType, scope, desc, count)
{
	m_OwnerClass = compilerModule->GetClass(*compilerModule->GetCompiler()->GetCompilerSymbol()->GetSymbolByTypeId(defineType.TypeId)).get();
	m_Data = m_OwnerClass->CreateVariableCopyClassMember(compilerModule, scope);
}

CompilerClassValue::~CompilerClassValue()
{
}

unsigned int CompilerClassValue::GetSize()
{
	return m_OwnerClass->GetDataSize();
}

const STDString& CompilerClassValue::GetOwnerClassName()
{
	return m_OwnerClass->GetName();
}

Share<CompilerValue> CompilerClassValue::GetMember(const STDString& name, STDString* nameSpace)
{
	auto index = GetMemberIndex(name, nameSpace);
	if (index < 0)
	{
		return nullptr;
	}

	return m_Data.size() > index  ? m_Data[index] : nullptr;
}

Share<CompilerValue> CompilerClassValue::GetMember(x_int32 index, bool indexFromSelf)
{
	if (indexFromSelf)
	{
		return m_Data[m_Data.size() - m_OwnerClass->GetClassMemberData().size() + index];
	}

	return m_Data[index];
}

int CompilerClassValue::GetMemberIndex(const STDString& memberName, STDString* nameSpace)
{
	return m_OwnerClass->GetMemberIndex(memberName, nameSpace);
}

int CompilerClassValue::GetMemberIndex(CompilerValue* value)
{
	x_uint64 index = 0;
	for (x_uint64 i = 0; i < m_Data.size(); i++)
	{
		if (m_Data[i].get() == value)
		{
			return (int)index;
		}

		index++;
	}

	return -1;
}

//bool CompilerClassValue::GetMemberName(const CompilerValue* memberValue, HString& outName)
//{
//	//return m_OwnerClass->GetMemberName(this, memberValue, outName, getOffset, offsets);
//
//	/*uint32 count = 0;
//	for (size_t i = 0; i < classValue->m_Data.size(); i++)
//	{
//		for (size_t j = 0; j < classValue->m_Data[i].second.size(); j++)
//		{
//			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, classValue->m_Data[i].second[j] }, value, outName, getOffset, offsets))
//			{
//				if (getOffset)
//				{
//					offsets->push_back({ m_Offsets[count], classValue->m_Data[i].second[j].get() });
//				}
//				return true;
//			}
//
//			count++;
//		}
//	}*/
//
//	return false;
//}