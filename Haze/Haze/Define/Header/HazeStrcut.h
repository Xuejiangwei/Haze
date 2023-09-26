#pragma once

#include "HazeToken.h"
#include "HazeValue.h"

enum class HazeSectionSignal : uint8
{
	Global,
	Local,
	Static,
	Class,
};

struct HazeDefineType
{
	HazeValueType PrimaryType;				//Type����

	HazeValueType SecondaryType;			//ָ��ָ������,�Զ�����ָ��ֵΪvoid

	HAZE_STRING CustomName;				//�Զ���������

	HazeDefineType() : PrimaryType(HazeValueType::Void)
	{
		SecondaryType = HazeValueType::Void;
	}

	~HazeDefineType()
	{
	}

	HazeDefineType(HazeValueType m_Type, const HAZE_STRING& CustomName) : PrimaryType(m_Type), SecondaryType(HazeValueType::Void)
	{
		this->CustomName = CustomName;
	}

	HazeDefineType(HazeValueType m_Type, const HAZE_CHAR* CustomName) : PrimaryType(m_Type), SecondaryType(HazeValueType::Void)
	{
		this->CustomName = CustomName;
	}

	bool operator==(const HazeDefineType& InType) const
	{
		return PrimaryType == InType.PrimaryType && SecondaryType == InType.SecondaryType
			&& CustomName == InType.CustomName;
	}

	bool operator!=(const HazeDefineType& InType)
	{
		return PrimaryType != InType.PrimaryType || SecondaryType != InType.SecondaryType
			|| CustomName != InType.CustomName;
	}

	void Reset()
	{
		PrimaryType = HazeValueType::Void;
		SecondaryType = HazeValueType::Void;
		CustomName.clear();
	}

	bool NeedSecondaryType() const { return NeedSecondaryType(*this); }

	bool NeedCustomName() const { return NeedCustomName(*this); }

	bool HasCustomName() const { return HasCustomName(*this); }

	bool StringStreamTo(HAZE_STRING_STREAM& SStream) const { return StringStreamTo(SStream, *this); }

	template<typename Class>
	void StringStream(Class* This, void(Class::* StringCall)(HAZE_STRING&), void(Class::* TypeCall)(uint32&)) { StringStream(This, StringCall, TypeCall, *this); }

	static bool NeedSecondaryType(const HazeDefineType& m_Type)
	{
		return m_Type.PrimaryType == HazeValueType::Array || m_Type.PrimaryType == HazeValueType::PointerBase ||
			m_Type.PrimaryType == HazeValueType::PointerFunction || m_Type.PrimaryType == HazeValueType::PointerArray ||
			m_Type.PrimaryType == HazeValueType::PointerPointer || m_Type.PrimaryType == HazeValueType::ReferenceBase;
	}

	static bool NeedCustomName(const HazeDefineType& m_Type)
	{
		return m_Type.PrimaryType == HazeValueType::Class || m_Type.PrimaryType == HazeValueType::PointerClass ||
			m_Type.SecondaryType == HazeValueType::Class || m_Type.SecondaryType == HazeValueType::PointerClass ||
			m_Type.PrimaryType == HazeValueType::ReferenceClass;
	}

	static bool HasCustomName(const HazeDefineType& m_Type)
	{
		return !m_Type.CustomName.empty();
	}

	static bool StringStreamTo(HAZE_STRING_STREAM& SStream, const HazeDefineType& m_Type)
	{
		SStream << CAST_TYPE(m_Type.PrimaryType);

		/*if (Type.PrimaryType == HazeValueType::MultiVariable)
		{
			HazeLog::LogInfo(HazeLog::Error, L"%s\n", L"Haze to do : " L"��ʱֻ��������Ļ�������");
		}*/

		if (m_Type.NeedSecondaryType())
		{
			SStream << " " << CAST_TYPE(m_Type.SecondaryType);
		}

		if (m_Type.NeedCustomName())
		{
			if (m_Type.HasCustomName())
			{
				SStream << " " << m_Type.CustomName;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	template<typename Class>
	static void StringStream(Class* This, void(Class::* StringCall)(HAZE_STRING&), void(Class::* TypeCall)(uint32&), HazeDefineType& m_Type)
	{
		(This->*TypeCall)((uint32&)m_Type.PrimaryType);

		/*if (Type.PrimaryType == HazeValueType::MultiVariable)
		{
			HazeLog::LogInfo(HazeLog::Error, L"%s\n", L"Haze to do : " L"��ʱֻд������Ļ�������");
		}*/

		if (m_Type.NeedSecondaryType())
		{
			(This->*TypeCall)((uint32&)m_Type.SecondaryType);
		}

		if (m_Type.NeedCustomName())
		{
			(This->*StringCall)(m_Type.CustomName);
		}
	}
};

struct HazeDefineTypeHashFunction
{
	uint64 operator()(const HazeDefineType& m_Type) const
	{
		if (!m_Type.CustomName.empty())
		{
			return std::hash<HAZE_STRING>()(m_Type.CustomName);
		}
		else
		{
			return (uint64)m_Type.PrimaryType * 100 + (uint64)m_Type.SecondaryType * 10;
		}
	}
};

struct HazeDefineVariable
{
	HazeDefineType Type;		//��������
	HAZE_STRING Name;			//������

	HazeDefineVariable() {}
	HazeDefineVariable(const HazeDefineType& type, const HAZE_STRING& name)
		: Type(type), Name(name) {}
};

struct HazeVariableData
{
	HazeDefineVariable Variable;
	int Offset;
	uint32 Size;
	uint32 Line;
};

struct HazeClassData
{
	std::vector<HazeDefineVariable> m_Data;
};