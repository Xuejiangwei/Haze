#include "HazeLog.h"

#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"

HazeCompilerClass::HazeCompilerClass(HazeCompilerModule* compilerModule, const HAZE_STRING& name, std::vector<HazeCompilerClass*>& parentClass,
	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& data)
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
	
	uint32 MemberNum = 0;
	std::shared_ptr<HazeCompilerValue> LastMember = nullptr;
	for (size_t i = 0; i < m_Data.size(); i++)
	{
		for (size_t j = 0; j < m_Data[i].second.size(); j++)
		{
			m_Data[i].second[j].second->Desc = m_Data[i].first;
			MemberNum++;
			LastMember = m_Data[i].second[j].second;
		}
	}

	MemoryAlign(MemberNum);
	uint32 AlignSize = GetAlignSize();
	AlignSize = AlignSize > HAZE_ALIGN_BYTE ? AlignSize : HAZE_ALIGN_BYTE;
	m_DataSize = LastMember ? HAZE_ALIGN(Vector_Offset.back() + LastMember->GetSize(), AlignSize) : 0;
	HAZE_LOG_INFO(HAZE_TEXT("¿‡<%s> DataSize %d\n"), name.c_str(), m_DataSize);
}

HazeCompilerClass::~HazeCompilerClass()
{
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerClass::FindFunction(const HAZE_STRING& m_FunctionName)
{
	auto Iter = m_HashMap_Functions.find(GetHazeClassFunctionName(m_Name, m_FunctionName));
	if (Iter != m_HashMap_Functions.end())
	{
		return m_Functions[Iter->second];
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerClass::AddFunction(std::shared_ptr<HazeCompilerFunction>& Function)
{
	auto Iter = m_HashMap_Functions.find(Function->GetName());
	if (Iter == m_HashMap_Functions.end())
	{
		m_Functions.push_back(Function);
		m_HashMap_Functions[Function->GetName()] = (unsigned int)m_Functions.size() - 1;

		return Function;
	}

	return m_Functions[Iter->second];
}

void HazeCompilerClass::InitThisValue()
{
	NewPointerToValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(CreateVariable(m_Module, HazeDefineVariable(HazeDefineType(HazeValueType::Class, m_Name),
		HAZE_TEXT("")), HazeVariableScope::None, HazeDataDesc::ClassThis, 0));
	ThisClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(CreateVariable(m_Module, HazeDefineVariable(HazeDefineType(HazeValueType::Class, m_Name),
		HAZE_CLASS_THIS), HazeVariableScope::Local, HazeDataDesc::ClassThis, 0));

	ThisPointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(CreateVariable(m_Module, HazeDefineVariable(HazeDefineType(HazeValueType::PointerClass, m_Name),
		HAZE_CLASS_THIS), HazeVariableScope::Local, HazeDataDesc::ClassThis, 0));
}

int HazeCompilerClass::GetMemberIndex(const HAZE_STRING& MemberName)
{
	size_t Index = 0;
	for (size_t i = 0; i < m_Data.size(); i++)
	{
		for (size_t j = 0; j < m_Data[i].second.size(); j++)
		{
			if (m_Data[i].second[j].first == MemberName)
			{
				return (int)Index;
			}

			Index++;
		}
	}

	return -1;
}

bool HazeCompilerClass::GetMemberName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	return GetMemberName(Value.get(), OutName);
}

bool HazeCompilerClass::GetMemberName(const HazeCompilerValue* Value, HAZE_STRING& OutName)
{
	for (size_t i = 0; i < ThisClassValue->m_Data.size(); i++)
	{
		for (size_t j = 0; j < ThisClassValue->m_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, ThisClassValue->m_Data[i].second[j] }, Value, OutName))
			{
				return true;
			}
		}
	}

	for (size_t i = 0; i < NewPointerToValue->m_Data.size(); i++)
	{
		for (size_t j = 0; j < NewPointerToValue->m_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, NewPointerToValue->m_Data[i].second[j] }, Value, OutName))
			{
				return true;
			}
		}
	}

	return false;
}

bool HazeCompilerClass::GetMemberName(HazeCompilerClassValue* ClassValue, const HazeCompilerValue* Value, HAZE_STRING& OutName)
{
	for (size_t i = 0; i < ClassValue->m_Data.size(); i++)
	{
		for (size_t j = 0; j < ClassValue->m_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, ClassValue->m_Data[i].second[j] }, Value, OutName))
			{
				return true;
			}
		}
	}

	return false;
}

void HazeCompilerClass::GenClassData_I_Code(HAZE_STRING_STREAM& SStream)
{
#if HAZE_I_CODE_ENABLE

	size_t DataNum = 0;
	for (auto& Datas : m_Data)
	{
		DataNum += Datas.second.size();
	}

	SStream << GetClassLabelHeader() << " " << m_Name << " " << GetDataSize() << " " << DataNum << std::endl;

	uint32 Index = 0;
	for (size_t i = 0; i < m_Data.size(); ++i)
	{
		for (size_t j = 0; j < m_Data[i].second.size(); j++)
		{
			SStream << m_Data[i].second[j].first << " " << CAST_TYPE(m_Data[i].second[j].second->GetValueType().PrimaryType);
			if (m_Data[i].second[j].second->GetValueType().NeedSecondaryType())
			{
				SStream << " " << CAST_TYPE(m_Data[i].second[j].second->GetValueType().SecondaryType);
			}
			else if (m_Data[i].second[j].second->GetValueType().NeedCustomName())
			{
				SStream << " " << m_Data[i].second[j].second->GetValueType().CustomName;
			}

			SStream << " " << Vector_Offset[Index] << " " << m_Data[i].second[j].second->GetSize() << std::endl;
			Index++;
		}
	}

#endif //HAZE_I_CODE_ENABLE
}

void HazeCompilerClass::GenClassFunction_I_Code(HAZE_STRING_STREAM& SStream)
{
#if HAZE_I_CODE_ENABLE

	for (auto& Iter : m_Functions)
	{
		Iter->GenI_Code(SStream);
	}

#endif //HAZE_I_CODE_ENABLE
}

uint32 HazeCompilerClass::GetDataSize()
{
	return m_DataSize;
}

uint32 HazeCompilerClass::GetAlignSize()
{
	uint32 MaxMemberSize = 0;

	for (size_t i = 0; i < m_ParentClass.size(); i++)
	{
		uint32 ParentAlignSize = m_ParentClass[i]->GetAlignSize();
		if (ParentAlignSize > MaxMemberSize)
		{
			MaxMemberSize = ParentAlignSize;
		}
	}

	for (auto& It : m_Data)
	{
		for (auto& Iter : It.second)
		{
			if (Iter.second->IsClass())
			{
				uint32 ClassAlignSize = std::dynamic_pointer_cast<HazeCompilerClassValue>(Iter.second)->GetOwnerClass()->GetAlignSize();
				if (ClassAlignSize > MaxMemberSize)
				{
					MaxMemberSize = ClassAlignSize;
				}
			}
			else
			{
				if (Iter.second->GetSize() > MaxMemberSize)
				{
					MaxMemberSize = Iter.second->GetSize();
				}
			}
		}
	}

	return MaxMemberSize;
}

uint32 HazeCompilerClass::GetOffset(uint32 Index, std::shared_ptr<HazeCompilerValue> Member)
{
	return Index * Member->GetSize();
}

void HazeCompilerClass::MemoryAlign(uint32 MemberNum)
{
	Vector_Offset.resize(MemberNum);
	uint32 Align = GetAlignSize();
	uint32 Index = 0;

	uint32 Size = 0;
	for (size_t i = 0; i < m_ParentClass.size(); i++)
	{
		Size += m_ParentClass[i]->GetDataSize();
	}

	for (auto& It : m_Data)
	{
		for (auto& Iter : It.second)
		{
			uint32 ModSize = Size % Align;

			if (ModSize + Iter.second->GetSize() <= Align || Iter.second->IsClass())
			{
				uint32 ModValue = ModSize % Iter.second->GetSize();
				if (ModValue == 0)
				{
					Vector_Offset[Index] = Size;
					Size += Iter.second->GetSize();
				}
				else
				{
					Vector_Offset[Index] = Size + ModValue;
					Size += (ModValue + Iter.second->GetSize());
				}
			}
			else
			{
				Vector_Offset[Index] = Size + (Align - ModSize);
				Size += ((Align - ModSize) + Iter.second->GetSize());

			}
			Index++;
		}
	}
}