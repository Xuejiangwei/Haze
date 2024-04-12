#pragma once

#include "HazeToken.h"
#include "HazeValue.h"
#include "HazeLog.h"

enum class HazeSectionSignal : uint8
{
	Global,
	Local,
	Static,
	Class,
};

struct HazeDefineType
{
	HazeValueType PrimaryType;				//Type类型

	HazeValueType SecondaryType;			//指针指向类型,自定义类指针值为void

	HAZE_STRING CustomName;				//自定义类型名

	HazeDefineType() : PrimaryType(HazeValueType::None), SecondaryType(HazeValueType::None)
	{
		CustomName.clear();
	}

	~HazeDefineType()
	{
	}

	HazeDefineType(HazeValueType type) : PrimaryType(type), SecondaryType(HazeValueType::None)
	{
		this->CustomName.clear();
	}

	HazeDefineType(HazeValueType type, const HAZE_STRING& customName) : PrimaryType(type), SecondaryType(HazeValueType::None)
	{
		this->CustomName = customName;
	}

	HazeDefineType(HazeValueType type, const HAZE_CHAR* customName) : PrimaryType(type), SecondaryType(HazeValueType::None)
	{
		this->CustomName = customName;
	}

	HazeDefineType(HazeValueType type, HazeValueType type2, const HAZE_STRING& customName) : PrimaryType(type), SecondaryType(type2)
	{
		this->CustomName = customName;
	}

	HazeDefineType(HazeValueType type, HazeValueType type2, const HAZE_CHAR* customName) : PrimaryType(type), SecondaryType(type2)
	{
		this->CustomName = customName;
	}

	bool operator==(const HazeDefineType& type) const
	{
		return PrimaryType == type.PrimaryType && SecondaryType == type.SecondaryType
			&& CustomName == type.CustomName;
	}

	bool operator!=(const HazeDefineType& type) const
	{
		return PrimaryType != type.PrimaryType || SecondaryType != type.SecondaryType
			|| CustomName != type.CustomName;
	}

	void Reset()
	{
		PrimaryType = HazeValueType::None;
		SecondaryType = HazeValueType::None;
		CustomName.clear();
	}

	bool NeedSecondaryType() const { return NeedSecondaryType(*this); }

	bool NeedCustomName() const { return NeedCustomName(*this); }

	bool HasCustomName() const { return HasCustomName(*this); }

	bool StringStreamTo(HAZE_STRING_STREAM& hss) const { return StringStreamTo(hss, *this); }

	void PointerTo(const HazeDefineType& type)
	{
		if (IsPointerPointer(type.PrimaryType))
		{
			HAZE_LOG_ERR_W("类型指向错误，指向指向指针的指针!\n");
		}
		else if (IsPointerType(type.PrimaryType))
		{
			PrimaryType = HazeValueType::PointerPointer;
			SecondaryType = type.SecondaryType;
			CustomName = type.CustomName;
		}
		else if (IsArrayType(type.PrimaryType))
		{
			switch (type.PrimaryType)
			{
			case HazeValueType::ArrayBase:
				PrimaryType = HazeValueType::PointerBase;
				break;
			case HazeValueType::ArrayClass:
				PrimaryType = HazeValueType::PointerClass;
				break;
			case HazeValueType::ArrayPointer:
				PrimaryType = HazeValueType::PointerPointer;
				break;
			default:
				break;
			}

			if (IsHazeDefaultTypeAndVoid(type.SecondaryType))
			{
				SecondaryType = type.SecondaryType;
				CustomName.clear();
			}
			else if (IsClassType(type.SecondaryType))
			{
				SecondaryType = HazeValueType::Class;
				CustomName = type.CustomName;
			}
			else
			{
				HAZE_LOG_ERR_W("类型指向数组类型错误!\n");
			}
		}
		else
		{
			if (IsHazeDefaultTypeAndVoid(type.PrimaryType))
			{
				PrimaryType = HazeValueType::PointerBase;
				SecondaryType = type.PrimaryType;
				CustomName.clear();
			}
			else if (IsClassType(type.PrimaryType))
			{
				PrimaryType = HazeValueType::PointerClass;
				SecondaryType = HazeValueType::None;
				CustomName = type.CustomName;
			}
			else
			{
				HAZE_LOG_ERR_W("类型指向错误!\n");
			}
		}
	}

	template<typename Class>
	void StringStream(Class* pThis, void(Class::* stringCall)(HAZE_STRING&), void(Class::* typeCall)(uint32&)) { StringStream(pThis, stringCall, typeCall, *this); }

	static bool NeedSecondaryType(const HazeDefineType& type)
	{
		return IsArrayType(type.PrimaryType) || type.PrimaryType == HazeValueType::PointerBase ||
			type.PrimaryType == HazeValueType::PointerFunction ||
			type.PrimaryType == HazeValueType::PointerPointer || type.PrimaryType == HazeValueType::ReferenceBase;
	}

	static bool NeedCustomName(const HazeDefineType& type)
	{
		return type.PrimaryType == HazeValueType::Class || type.PrimaryType == HazeValueType::PointerClass ||
			type.SecondaryType == HazeValueType::Class || type.SecondaryType == HazeValueType::PointerClass ||
			type.PrimaryType == HazeValueType::ReferenceClass;
	}

	static bool HasCustomName(const HazeDefineType& type)
	{
		return !type.CustomName.empty();
	}

	static bool StringStreamTo(HAZE_STRING_STREAM& hss, const HazeDefineType& type)
	{
		hss << CAST_TYPE(type.PrimaryType);

		/*if (Type.PrimaryType == HazeValueType::MultiVariable)
		{
			HazeLog::LogInfo(HazeLog::Error, L"%s\n", L"Haze to do : " L"暂时只读多参数的基本类型");
		}*/

		if (type.NeedSecondaryType())
		{
			hss << " " << CAST_TYPE(type.SecondaryType);
		}

		if (type.NeedCustomName())
		{
			if (type.HasCustomName())
			{
				hss << " " << type.CustomName;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	template<typename Class>
	static void StringStream(Class* pThis, void(Class::* stringCall)(HAZE_STRING&), void(Class::* typeCall)(uint32&), HazeDefineType& type)
	{
		(pThis->*typeCall)((uint32&)type.PrimaryType);

		/*if (Type.PrimaryType == HazeValueType::MultiVariable)
		{
			HazeLog::LogInfo(HazeLog::Error, L"%s\n", L"Haze to do : " L"暂时只写多参数的基本类型");
		}*/

		if (type.NeedSecondaryType())
		{
			(pThis->*typeCall)((uint32&)type.SecondaryType);
		}

		if (type.NeedCustomName())
		{
			(pThis->*stringCall)(type.CustomName);
		}
	}
};

struct HazeDefineTypeHashFunction
{
	uint64 operator()(const HazeDefineType& type) const
	{
		if (!type.CustomName.empty())
		{
			return std::hash<HAZE_STRING>()(type.CustomName);
		}
		else
		{
			return (uint64)type.PrimaryType * 100 + (uint64)type.SecondaryType * 10;
		}
	}
};

struct HazeDefineVariable
{
	HazeDefineType Type;		//变量类型
	HAZE_STRING Name;			//变量名

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
	std::vector<HazeDefineVariable> Data;
};