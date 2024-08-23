#pragma once

#include "HazeToken.h"
#include "HazeValue.h"
#include "HazeLog.h"

#ifdef HAZE
	#include <sstream>
	#include "JwHeader.h"
#endif

enum class HazeSectionSignal : uint8
{
	Global,
	Local,
	Static,
	Class,
	Enum,
};

struct HazeDefineType
{
	HazeValueType PrimaryType;				//Type类型
	HazeValueType SecondaryType;
	const HString* CustomName;

	HazeDefineType() : PrimaryType(HazeValueType::None), SecondaryType(HazeValueType::None), CustomName(nullptr)
	{
	}

	~HazeDefineType()
	{
	}

	HazeDefineType(HazeValueType type) : PrimaryType(type), SecondaryType(HazeValueType::None), CustomName(nullptr)
	{
		CheckValid();
	}

	HazeDefineType(HazeValueType type, HazeValueType type2) : PrimaryType(type), SecondaryType(type2), CustomName(nullptr)
	{
		CheckValid();
	}

	HazeDefineType(HazeValueType type, const HString* name) : PrimaryType(type), SecondaryType(HazeValueType::None), CustomName(name)
	{
		CheckValid();
	}

	HazeDefineType(HazeValueType type, HazeValueType type2, const HString* name) : PrimaryType(type), SecondaryType(type2), CustomName(name)
	{
		CheckValid();
	}

	void CheckValid() const
	{
		if (IsHazeBaseTypeAndVoid(PrimaryType)) {}
		else if (IsEnumType(PrimaryType) && IsIntegerType(SecondaryType)) {}
		else if (IsRefrenceType(PrimaryType) && IsHazeBaseType(SecondaryType)) {}
		else if (IsArrayType(PrimaryType) && IsHazeBaseType(SecondaryType)) {}
		else if (IsArrayType(PrimaryType) && IsClassType(SecondaryType) && CustomName && !CustomName->empty()) {}
		else if (IsClassType(PrimaryType) && IsNoneType(SecondaryType) && CustomName && !CustomName->empty()) {}
		else if (IsStringType(PrimaryType) && IsNoneType(SecondaryType) && !CustomName) {}
		else if (IsMultiVariableTye(PrimaryType) && IsNoneType(SecondaryType) && !CustomName) {}
		else
		{
			HAZE_LOG_ERR_W("基本类型指针的类型错误<%s><%s><%s>\n", GetHazeValueTypeString(PrimaryType), GetHazeValueTypeString(SecondaryType),
				CustomName ? CustomName->empty() ? H_TEXT("") : CustomName->c_str() : H_TEXT(""));
		}
	}

	bool operator==(const HazeDefineType& type) const 
	{ return type.PrimaryType == PrimaryType && type.SecondaryType == SecondaryType && type.CustomName == CustomName; }

	bool operator!=(const HazeDefineType& type) const { return !(type == *this); }

	uint32 GetTypeSize() const { return GetSizeByHazeType(PrimaryType); }

	void Reset() { PrimaryType = HazeValueType::None; SecondaryType = HazeValueType::None, CustomName = nullptr; }

	bool NeedSecondaryType() const { return IsArrayType(PrimaryType) || IsEnumType(PrimaryType) || IsRefrenceType(PrimaryType); }

	bool NeedCustomName() const { return IsClassType(PrimaryType) || (IsArrayType(PrimaryType) && IsClassType(SecondaryType)); }

	bool StringStreamTo(HAZE_STRING_STREAM& hss) const { return StringStreamTo(hss, *this); }

	bool IsStrongerType(const HazeDefineType& type) const
	{
		if (*this != type)
		{
			if (type.PrimaryType != PrimaryType)
			{
				auto strongerType = GetStrongerType(PrimaryType, type.PrimaryType, false);
				return strongerType == HazeValueType::None ? false : strongerType == PrimaryType;
			}
			else
			{
				return CustomName == type.CustomName;
			}
		}

		return false;
	}

	HString GetFullTypeName() const
	{
		HString fullName;

		if (CustomName)
		{
			fullName = *CustomName;
		}
		else
		{
			if (!NeedSecondaryType())
			{
				fullName = GetHazeValueTypeString(PrimaryType);
			}
			else
			{
				HAZE_LOG_ERR_W("获得类型<%s><%s><%s>的全长名错误\n", GetHazeValueTypeString(PrimaryType), 
					GetHazeValueTypeString(SecondaryType),
					CustomName ? CustomName->empty() ? H_TEXT("") : CustomName->c_str() : H_TEXT(""));
			}
		}
		
		return fullName;
	}

	void UpToRefrence()
	{
		SecondaryType = PrimaryType;
		PrimaryType = HazeValueType::Refrence;
	}

	void UpTypeToRefrence(const HazeDefineType& type)
	{
		*this = type;
		if (IsHazeBaseType(PrimaryType))
		{
			UpToRefrence();
		}
	}

	void UpToArray()
	{
		SecondaryType = PrimaryType;
		PrimaryType = HazeValueType::Array;
	}

	void Pointer()
	{
		PrimaryType = HazeValueType::UInt64;
		SecondaryType = HazeValueType::None;
		CustomName = nullptr;
	}

	template<typename Class>
	void StringStream(Class* pThis, void(Class::* stringCall)(const HString*&), void(Class::* typeCall)(uint32&)) { StringStream(pThis, stringCall, typeCall, *this); }

	/*bool HasCustomName(const HazeDefineType& type)
	{
		return  type.CustomName && !type.CustomName->empty();
	}*/

	static bool StringStreamTo(HAZE_STRING_STREAM& hss, const HazeDefineType& type)
	{
		if (IsEnumType(type.PrimaryType))
		{
			hss << CAST_TYPE(type.SecondaryType);
			return true;
		}

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
			if (type.CustomName)
			{
				hss << " " << *type.CustomName;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	template<typename Class>
	static void StringStream(Class* pThis, void(Class::* stringCall)(const HString*&), void(Class::* typeCall)(uint32&), HazeDefineType& type)
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
		if (type.CustomName && !type.CustomName->empty())
		{
			return std::hash<HString>()(*type.CustomName);
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
	HString Name;			//变量名

	HazeDefineVariable() {}
	HazeDefineVariable(const HazeDefineType& type, const HString& name)
		: Type(type), Name(name) {}
};

struct HazeVariableData
{
	HazeDefineVariable Variable;
	int Offset;
	uint32 Size;
	uint32 Line;
};

struct HazeTempRegisterData
{
	HString Name;
	uint32 Offset;
	HazeDefineType Type;
};

struct TemplateDefineTypes
{
	V_Array<struct TemplateDefineType> Types;

	TemplateDefineTypes()
	{
		Types.clear();
	}
};

struct TemplateDefineType
{
	bool IsDefines;
	Share<TemplateDefineTypes> Defines;
	Share<HazeDefineType> Type;

	TemplateDefineType(bool isDefines, Share<TemplateDefineTypes> defineTypes, Share<HazeDefineType> type) : IsDefines(isDefines)
	{
		if (IsDefines)
		{
			Defines = defineTypes;
		}
		else
		{
			Type = type;
		}
	}

	TemplateDefineType(const TemplateDefineType& type)
		: TemplateDefineType(type.IsDefines, type.Defines, type.Type)
	{}
		
	TemplateDefineType& operator=(const TemplateDefineType& type)
	{
		if (IsDefines)
		{
			Defines = type.Defines;
		}
		else
		{
			Type = type.Type;
		}

		return *this;
	}

	~TemplateDefineType()
	{
	}
};