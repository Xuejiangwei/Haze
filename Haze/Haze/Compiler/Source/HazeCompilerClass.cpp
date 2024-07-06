#include "HazePch.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"

HazeCompilerClass::HazeCompilerClass(HazeCompilerModule* compilerModule, const HString& name, V_Array<HazeCompilerClass*>& parentClass,
	V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<HazeCompilerValue>>>>>& data)
	: m_Module(compilerModule), m_Name(name), m_ParentClass(std::move(parentClass)), m_Data(std::move(data))
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
	Share<HazeCompilerValue> lastMember = nullptr;
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

HazeCompilerClass::~HazeCompilerClass()
{
}

Share<HazeCompilerFunction> HazeCompilerClass::FindFunction(const HString& functionName)
{
	auto iter = m_HashMap_Functions.find(GetHazeClassFunctionName(m_Name, functionName));
	if (iter != m_HashMap_Functions.end())
	{
		return m_Functions[iter->second];
	}

	return nullptr;
}

Share<HazeCompilerFunction> HazeCompilerClass::AddFunction(Share<HazeCompilerFunction>& function)
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

void HazeCompilerClass::InitThisValue()
{
	m_NewPointerToValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(CreateVariable(m_Module, HazeDefineVariable(HazeDefineType(HazeValueType::Class, m_Name),
		H_TEXT("")), HazeVariableScope::None, HazeDataDesc::ClassThis, 0));
	
	m_ThisClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(CreateVariable(m_Module, HazeDefineVariable(HazeDefineType(HazeValueType::Class, m_Name),
		HAZE_CLASS_THIS), HazeVariableScope::Local, HazeDataDesc::ClassThis, 0));

	m_ThisPointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(CreateVariable(m_Module, HazeDefineVariable(HazeDefineType(HazeValueType::PointerClass, m_Name),
		HAZE_CLASS_THIS), HazeVariableScope::Local, HazeDataDesc::ClassThis, 0));
}

int HazeCompilerClass::GetMemberIndex(const HString& memberName)
{
	size_t index = 0;
	for (size_t i = 0; i < m_Data.size(); i++)
	{
		for (size_t j = 0; j < m_Data[i].second.size(); j++)
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

bool HazeCompilerClass::GetMemberName(const Share<HazeCompilerValue>& value, HString& outName)
{
	return GetMemberName(value.get(), outName);
}

bool HazeCompilerClass::GetMemberName(const HazeCompilerValue* value, HString& outName)
{
	for (size_t i = 0; i < m_ThisClassValue->m_Data.size(); i++)
	{
		for (size_t j = 0; j < m_ThisClassValue->m_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, m_ThisClassValue->m_Data[i].second[j] }, value, outName))
			{
				return true;
			}
		}
	}

	for (size_t i = 0; i < m_NewPointerToValue->m_Data.size(); i++)
	{
		for (size_t j = 0; j < m_NewPointerToValue->m_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, m_NewPointerToValue->m_Data[i].second[j] }, value, outName))
			{
				return true;
			}
		}
	}

	return false;
}

bool HazeCompilerClass::GetMemberName(HazeCompilerClassValue* classValue, const HazeCompilerValue* value, HString& outName)
{
	for (size_t i = 0; i < classValue->m_Data.size(); i++)
	{
		for (size_t j = 0; j < classValue->m_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, classValue->m_Data[i].second[j] }, value, outName))
			{
				return true;
			}
		}
	}

	return false;
}

void HazeCompilerClass::GenClassData_I_Code(HAZE_STRING_STREAM& hss)
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
			hss << m_Data[i].second[j].first << " " << CAST_TYPE(m_Data[i].second[j].second->GetValueType().PrimaryType);
			if (m_Data[i].second[j].second->GetValueType().NeedSecondaryType())
			{
				hss << " " << CAST_TYPE(m_Data[i].second[j].second->GetValueType().SecondaryType);
			}
			else if (m_Data[i].second[j].second->GetValueType().NeedCustomName())
			{
				hss << " " << m_Data[i].second[j].second->GetValueType().CustomName;
			}

			hss << " " << m_Offsets[index] << " " << m_Data[i].second[j].second->GetSize() << std::endl;
			index++;
		}
	}
}

void HazeCompilerClass::GenClassFunction_I_Code(HAZE_STRING_STREAM& hss)
{
	for (auto& iter : m_Functions)
	{
		iter->GenI_Code(hss);
	}
}

uint32 HazeCompilerClass::GetDataSize()
{
	return m_DataSize;
}

uint32 HazeCompilerClass::GetAlignSize()
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
				uint32 classAlignSize = std::dynamic_pointer_cast<HazeCompilerClassValue>(iter.second)->GetOwnerClass()->GetAlignSize();
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

uint32 HazeCompilerClass::GetOffset(uint32 index, Share<HazeCompilerValue> member)
{
	return index * member->GetSize();
}

int HazeCompilerClass::GetClassInheritLevel() const
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

void HazeCompilerClass::MemoryAlign(uint32 memberNum)
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