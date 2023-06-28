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
	HazeValueType PrimaryType;				//Type类型

	HazeValueType SecondaryType;			//指针指向类型,自定义类指针值为void

	HAZE_STRING CustomName;				//自定义类型名

	HazeDefineType() : PrimaryType(HazeValueType::Void)
	{
		SecondaryType = HazeValueType::Void;
	}

	~HazeDefineType()
	{
	}

	HazeDefineType(HazeValueType Type, const HAZE_STRING& CustomName) : PrimaryType(Type), SecondaryType(HazeValueType::Void)
	{
		this->CustomName = CustomName;
	}

	HazeDefineType(HazeValueType Type, const HAZE_CHAR* CustomName) : PrimaryType(Type), SecondaryType(HazeValueType::Void)
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

	static bool NeedSecondaryType(const HazeDefineType& Type)
	{
		return Type.PrimaryType == HazeValueType::Array || Type.PrimaryType == HazeValueType::PointerBase ||
			Type.PrimaryType == HazeValueType::PointerFunction || Type.PrimaryType == HazeValueType::PointerArray ||
			Type.PrimaryType == HazeValueType::PointerPointer || Type.PrimaryType == HazeValueType::ReferenceBase;
	}

	static bool NeedCustomName(const HazeDefineType& Type)
	{
		return Type.PrimaryType == HazeValueType::Class || Type.PrimaryType == HazeValueType::PointerClass ||
			Type.SecondaryType == HazeValueType::Class || Type.SecondaryType == HazeValueType::PointerClass ||
			Type.PrimaryType == HazeValueType::ReferenceClass;
	}

	static bool HasCustomName(const HazeDefineType& Type)
	{
		return !Type.CustomName.empty();
	}

	static bool StringStreamTo(HAZE_STRING_STREAM& SStream, const HazeDefineType& Type)
	{
		SStream << CAST_TYPE(Type.PrimaryType);

		/*if (Type.PrimaryType == HazeValueType::MultiVariable)
		{
			HazeLog::LogInfo(HazeLog::Error, L"%s\n", L"Haze to do : " L"暂时只读多参数的基本类型");
		}*/

		if (Type.NeedSecondaryType())
		{
			SStream << " " << CAST_TYPE(Type.SecondaryType);
		}

		if (Type.NeedCustomName())
		{
			if (Type.HasCustomName())
			{
				SStream << " " << Type.CustomName;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	template<typename Class>
	static void StringStream(Class* This, void(Class::* StringCall)(HAZE_STRING&), void(Class::* TypeCall)(uint32&), HazeDefineType& Type)
	{
		(This->*TypeCall)((uint32&)Type.PrimaryType);

		/*if (Type.PrimaryType == HazeValueType::MultiVariable)
		{
			HazeLog::LogInfo(HazeLog::Error, L"%s\n", L"Haze to do : " L"暂时只写多参数的基本类型");
		}*/

		if (Type.NeedSecondaryType())
		{
			(This->*TypeCall)((uint32&)Type.SecondaryType);
		}

		if (Type.NeedCustomName())
		{
			(This->*StringCall)(Type.CustomName);
		}
	}
};

struct HazeDefineTypeHashFunction
{
	uint64 operator()(const HazeDefineType& Type) const
	{
		if (!Type.CustomName.empty())
		{
			return std::hash<HAZE_STRING>()(Type.CustomName);
		}
		else
		{
			return (uint64)Type.PrimaryType * 100 + (uint64)Type.SecondaryType * 10;
		}
	}
};

struct HazeDefineVariable
{
	HazeDefineType Type;		//变量类型
	HAZE_STRING Name;			//变量名

	HazeDefineVariable() {}
	HazeDefineVariable(const HazeDefineType& Type, const HAZE_STRING& Name) : Type(Type), Name(Name) {}
};

struct HazeVariableData
{
	HazeDefineVariable Variable;
	int Offset;
	uint32 Size;
};

struct HazeClassData
{
	std::vector<HazeDefineVariable> Vector_Data;
};