#include "HazePch.h"
#include "CompilerClass.h"
#include "CompilerModule.h"
#include "CompilerFunction.h"
#include "CompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerClassValue.h"

CompilerClass::CompilerClass(CompilerModule* compilerModule, const HString& name, V_Array<CompilerClass*>& parentClass,
	V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<CompilerValue>>>>>& data)
	: m_Module(compilerModule), m_Name(name), m_ParentClass(Move(parentClass)), m_Data(Move(data))
{
	m_DataSize = 0;
	if (this->m_ParentClass.size() > 0)
	{
		for (size_t i = 0; i < this->m_ParentClass.size(); i++)
		{
			m_DataSize += this->m_ParentClass[i]->GetDataSize();
		}
	}
	
	uint32 memberNum = 0;
	Share<CompilerValue> lastMember = nullptr;
	for (size_t i = 0; i < m_Data.size(); i++)
	{
		for (size_t j = 0; j < m_Data[i].second.size(); j++)
		{
			m_Data[i].second[j].second->m_Desc = m_Data[i].first;
			memberNum++;
			lastMember = m_Data[i].second[j].second;
		}
	}

	MemoryAlign(memberNum);
	uint32 alignSize = GetAlignSize();
	alignSize = alignSize > HAZE_ALIGN_BYTE ? alignSize : HAZE_ALIGN_BYTE;
	m_DataSize = lastMember ? HAZE_ALIGN(m_Offsets.back() + lastMember->GetSize(), alignSize) : 0;
	HAZE_LOG_INFO(H_TEXT("¿‡<%s> DataSize %d\n"), name.c_str(), m_DataSize);
}

CompilerClass::~CompilerClass()
{
}

Share<CompilerFunction> CompilerClass::FindFunction(const HString& functionName)
{
	auto iter = m_HashMap_Functions.find(GetHazeClassFunctionName(m_Name, functionName));
	if (iter != m_HashMap_Functions.end())
	{
		return m_Functions[iter->second];
	}

	return nullptr;
}

Share<CompilerFunction> CompilerClass::AddFunction(Share<CompilerFunction>& function)
{
	auto iter = m_HashMap_Functions.find(function->GetName());
	if (iter == m_HashMap_Functions.end())
	{
		m_Functions.push_back(function);
		m_HashMap_Functions[function->GetName()] = (unsigned int)m_Functions.size() - 1;

		return function;
	}

	return m_Functions[iter->second];
}

void CompilerClass::InitThisValue()
{
	/*m_ThisClassValue = DynamicCast<HazeCompilerClassValue>(CreateVariable(m_Module, 
		HazeDefineVariable(HazeDefineType(HazeValueType::Class, &m_Name),
		TOKEN_THIS), HazeVariableScope::Local, HazeDataDesc::ClassThis, 0));*/
}

int CompilerClass::GetMemberIndex(const HString& memberName)
{
	uint64 index = 0;
	for (uint64 i = 0; i < m_Data.size(); i++)
	{
		for (uint64 j = 0; j < m_Data[i].second.size(); j++)
		{
			if (m_Data[i].second[j].first == memberName)
			{
				return (int)index;
			}

			index++;
		}
	}

	return -1;
}

int CompilerClass::GetMemberIndex(CompilerValue* value)
{
	uint64 index = 0;
	for (uint64 i = 0; i < m_Data.size(); i++)
	{
		for (uint64 j = 0; j < m_Data[i].second.size(); j++)
		{
			if (m_Data[i].second[j].second.get() == value)
			{
				return (int)index;
			}

			index++;
		}
	}

	return -1;
}

//bool HazeCompilerClass::GetThisMemberName(const HazeCompilerValue* value, HString& outName, bool getOffset, V_Array<uint64>* offsets)
//{
//	uint32 count = 0;
//	for (size_t i = 0; i < m_ThisClassValue->m_Data.size(); i++)
//	{
//		for (size_t j = 0; j < m_ThisClassValue->m_Data[i].second.size(); j++)
//		{
//			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, m_ThisClassValue->m_Data[i].second[j] },
//				value, outName, getOffset, offsets))
//			{
//				if (getOffset)
//				{
//					offsets->push_back(m_Offsets[count]);
//				}
//				return true;
//			}
//
//			count++;
//		}
//	}
//
//	return false;
//}

bool CompilerClass::GetMemberName(CompilerClassValue* classValue, const CompilerValue* value, HString& outName, bool getOffset, 
	V_Array<Pair<uint64, CompilerValue*>>* offsets)
{
	uint32 count = 0;
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
	}

	return false;
}

const CompilerValue* CompilerClass::GetMemberValue(uint64 index)
{
	uint64 counter = 0;
	for (uint64 i = 0; i < m_Data.size(); i++)
	{
		for (uint64 j = 0; j < m_Data[i].second.size(); j++)
		{
			if (index == counter)
			{
				return m_Data[i].second[j].second.get();
			}

			counter++;
		}
	}

	return nullptr;
}

void CompilerClass::GenClassData_I_Code(HAZE_STRING_STREAM& hss)
{
	size_t dataNum = 0;
	for (auto& Datas : m_Data)
	{
		dataNum += Datas.second.size();
	}

	hss << GetClassLabelHeader() << " " << m_Name << " " << GetDataSize() << " " << dataNum << std::endl;

	uint32 index = 0;
	for (size_t i = 0; i < m_Data.size(); ++i)
	{
		for (size_t j = 0; j < m_Data[i].second.size(); j++)
		{
			hss << m_Data[i].second[j].first << " ";
			m_Data[i].second[j].second->GetValueType().StringStreamTo(hss);

			hss << " " << m_Offsets[index] << " " << m_Data[i].second[j].second->GetSize() << std::endl;
			index++;
		}
	}
}

void CompilerClass::GenClassFunction_I_Code(HAZE_STRING_STREAM& hss)
{
	for (auto& iter : m_Functions)
	{
		iter->GenI_Code(hss);
	}
}

uint32 CompilerClass::GetDataSize()
{
	return m_DataSize;
}

uint32 CompilerClass::GetAlignSize()
{
	uint32 maxMemberSize = 0;

	for (size_t i = 0; i < m_ParentClass.size(); i++)
	{
		uint32 parentAlignSize = m_ParentClass[i]->GetAlignSize();
		if (parentAlignSize > maxMemberSize)
		{
			maxMemberSize = parentAlignSize;
		}
	}

	for (auto& it : m_Data)
	{
		for (auto& iter : it.second)
		{
			if (iter.second->IsClass())
			{
				uint32 classAlignSize = DynamicCast<CompilerClassValue>(iter.second)->GetOwnerClass()->GetAlignSize();
				if (classAlignSize > maxMemberSize)
				{
					maxMemberSize = classAlignSize;
				}
			}
			else
			{
				if (iter.second->GetSize() > maxMemberSize)
				{
					maxMemberSize = iter.second->GetSize();
				}
			}
		}
	}

	return maxMemberSize;
}

uint32 CompilerClass::GetOffset(uint32 index, Share<CompilerValue> member)
{
	return index * member->GetSize();
}

int CompilerClass::GetClassInheritLevel() const
{
	uint32 maxLevel = 0;
	uint32 inheritLevel = 0;
	for (auto& parentClass : m_ParentClass)
	{
		inheritLevel = parentClass->GetClassInheritLevel() + 1;
		if (inheritLevel > maxLevel)
		{
			maxLevel = inheritLevel;
		}
	}

	return maxLevel;
}

void CompilerClass::MemoryAlign(uint32 memberNum)
{
	m_Offsets.resize(memberNum);
	uint32 align = GetAlignSize();
	uint32 index = 0;

	uint32 size = 0;
	for (size_t i = 0; i < m_ParentClass.size(); i++)
	{
		size += m_ParentClass[i]->GetDataSize();
	}

	for (auto& it : m_Data)
	{
		for (auto& iter : it.second)
		{
			uint32 modSize = size % align;

			if (modSize + iter.second->GetSize() <= align || iter.second->IsClass())
			{
				uint32 modValue = modSize % iter.second->GetSize();
				if (modValue == 0)
				{
					m_Offsets[index] = size;
					size += iter.second->GetSize();
				}
				else
				{
					m_Offsets[index] = size + modValue;
					size += (modValue + iter.second->GetSize());
				}
			}
			else
			{
				m_Offsets[index] = size + (align - modSize);
				size += ((align - modSize) + iter.second->GetSize());

			}
			index++;
		}
	}
}