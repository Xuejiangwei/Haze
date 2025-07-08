#pragma once

#include "HazeToken.h"
#include "HazeValue.h"
#include "HazeLog.h"

#ifdef HAZE
	#include <sstream>
	#include "JwHeader.h"
#endif

#define HAZE_VAR_BASE_TYPE(TYPE) HazeVariableType(TYPE, HAZE_TYPE_ID(TYPE))
#define HAZE_COMPLEX_BASE_CAST(INFO) ((HazeComplexTypeInfoBase*)(&INFO))

enum class HazeSectionSignal : x_uint8
{
	Global,
	Local,
	Static,
	Class,
	Enum,
	Closure,
};


struct HazeVariableType
{
	HazeValueType BaseType;
	x_uint32 TypeId;

	HazeVariableType() { SetBaseTypeAndId(HazeValueType::None); }

	explicit HazeVariableType(HazeValueType type) { SetBaseTypeAndId(type); }

	explicit HazeVariableType(HazeValueType type, x_uint32 typeId) : BaseType(type), TypeId(typeId) {}

	bool operator==(const HazeVariableType& type) const { return type.BaseType == BaseType && type.TypeId == TypeId; }

	bool operator!=(const HazeVariableType& type) const { return !(type == *this); }

	x_uint32 GetTypeSize() const { return GetSizeByHazeType(BaseType); }

	void Reset() { BaseType = HazeValueType::None; TypeId = 0; }

	void SetBaseTypeAndId(HazeValueType type)
	{
		if (IsHazeBaseTypeAndVoid(type))
		{
			BaseType = type; TypeId = HAZE_TYPE_ID(type);
		}
		else
		{
			HAZE_LOG_ERR_W("设置类型错误");
		}
	}

	bool StringStreamTo(HAZE_STRING_STREAM& hss) const { return StringStreamTo(hss, *this); }

	bool IsStrongerType(const HazeVariableType& type) const
	{
		if (*this != type)
		{
			if (type.BaseType != BaseType)
			{
				auto strongerType = GetStrongerType(BaseType, type.BaseType, false);
				return strongerType == HazeValueType::None ? false : strongerType == BaseType;
			}
		}

		return false;
	}

	void UpToRefrence()
	{
		TypeId = HAZE_TYPE_ID(BaseType);
		BaseType = HazeValueType::Refrence;
	}

	void UpTypeToRefrence(const HazeVariableType& type)
	{
		*this = type;
		if (IsHazeBaseType(BaseType))
		{
			UpToRefrence();
		}
	}

	void Pointer()
	{
		BaseType = HazeValueType::UInt64;
		TypeId = HAZE_TYPE_ID(BaseType);
	}

	void DynamicClassUnknow()
	{
		BaseType = HazeValueType::DynamicClassUnknow;
		TypeId = HAZE_TYPE_ID(BaseType);
	}

	template<typename Class>
	void StringStream(Class* pThis, void(Class::* stringCall)(const HString*&), void(Class::* typeCall)(x_uint32&)) { StringStream(pThis, stringCall, typeCall, *this); }

	/*bool HasCustomName(const HazeDefineType& type)
	{
		return  type.CustomName && !type.CustomName->empty();
	}*/

	template<typename Class>
	static HazeVariableType StringStreamFrom(HAZE_IFSTREAM& stream, Class* pThis, const HString*(Class::* stringCall)(const HString&))
	{
		HazeVariableType type;
		stream >> *(x_uint32*)(&type.BaseType);
		stream >> *(x_uint32*)(&type.TypeId);
		return type;
	}

	static bool StringStreamTo(HAZE_STRING_STREAM& hss, const HazeVariableType& type)
	{
		hss << CAST_TYPE(type.BaseType) << " " << type.TypeId;
		return true;
	}

	template<typename Class>
	static void StringStream(Class* pThis, void(Class::* stringCall)(const HString*&), void(Class::* typeCall)(x_uint32&), HazeVariableType& type)
	{
		(pThis->*typeCall)((x_uint32&)type.BaseType);
		(pThis->*typeCall)((x_uint32&)type.TypeId);
	}

	static const HazeVariableType& VoidType()
	{
		static HazeVariableType s_Type(HazeValueType::Void);
		return s_Type;
	}

	static const HazeVariableType& StringType()
	{
		static HazeVariableType s_Type(HazeValueType::String);
		return s_Type;
	}
};

struct HazeNewDefineType
{
	HazeVariableType BaseType;
	x_uint64 ArrayDimension;
};

struct HazeDefineVariable
{
	HazeVariableType Type;		//变量类型
	HString Name;				//变量名

	HazeDefineVariable() : Type(HazeValueType::None) {}
	HazeDefineVariable(const HazeVariableType& type, const HString& name)
		: Type(type), Name(name) {}
};

struct HazeVariableData
{
	HazeDefineVariable Variable;
	int Offset;
	x_uint32 Size;
	x_uint32 Line;
};

struct HazeTempRegisterData
{
	HString Name;
	x_uint32 Offset;
	HazeVariableType Type;
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
	Share<HazeNewDefineType> Type;

	TemplateDefineType() : IsDefines(false)
	{
	}

	TemplateDefineType(bool isDefines, Share<TemplateDefineTypes> defineTypes, Share<HazeNewDefineType> type) : IsDefines(isDefines)
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

	/*~TemplateDefineType()
	{
	}*/
};

struct HazFrameFunctionData
{
	V_Array<HazeDefineVariable*> LocalParams;
};