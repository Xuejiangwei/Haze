#pragma once

#include "HazeToken.h"
#include "HazeValue.h"

enum class HazeSectionSignal : unsigned __int8
{
	Global,
	Function,
	Class,
	StandardLibrary,
};

struct HazeDefineType
{
	HazeValueType PrimaryType;				//Type����
	
	HazeValueType SecondaryType;			//ָ��ָ������,�Զ�����ָ��ֵΪvoid

	HAZE_STRING CustomName;				//�Զ���������
	bool IsArray;			
	

	HazeDefineType() : PrimaryType(HazeValueType::Void)
	{
		SecondaryType = HazeValueType::Void;
		IsArray = false;
	}

	~HazeDefineType()
	{
	}

	HazeDefineType(HazeValueType Type, const HAZE_STRING& CustomName) : PrimaryType(Type), SecondaryType(HazeValueType::Void), IsArray(false)
	{
		this->CustomName = CustomName;
	}

	HazeDefineType(HazeValueType Type, const HAZE_CHAR* CustomName) : PrimaryType(Type), SecondaryType(HazeValueType::Void), IsArray(false)
	{
		this->CustomName = CustomName;
	}

	bool operator==(const HazeDefineType& InType)
	{
		return PrimaryType == InType.PrimaryType && SecondaryType == InType.SecondaryType 
			&& CustomName == InType.CustomName && IsArray == InType.IsArray;
	}

	bool operator!=(const HazeDefineType& InType)
	{
		return PrimaryType != InType.PrimaryType || SecondaryType != InType.SecondaryType
			|| CustomName != InType.CustomName || IsArray != InType.IsArray;
	}
};

struct HazeDefineVariable
{
	HazeDefineType Type;		//��������
	HAZE_STRING Name;			//������

	HazeDefineVariable() {}
	HazeDefineVariable(const HazeDefineType& Type, const HAZE_STRING& Name) : Type(Type), Name(Name) {}
};

struct HazeLocalVariable
{
	HazeDefineVariable Variable;
	int Offset;
	uint32 Size;
};

struct HazeClassData
{
	std::vector<HazeDefineVariable> Vector_Data;
};