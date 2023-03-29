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
	HazeValueType PrimaryType;								//Type����
	

	//���Ż�
	HazeValueType PointerToType;				//ָ��ָ������
	HAZE_STRING CustomName;				//�Զ���������

	HazeDefineType() : PrimaryType(HazeValueType::Void)
	{
		PointerToType = HazeValueType::Void;
	}

	~HazeDefineType()
	{
	}

	HazeDefineType(HazeValueType Type, const HAZE_STRING& CustomName) : PrimaryType(Type), CustomName(CustomName.c_str())
	{
	}

	HazeDefineType(HazeValueType Type, const HAZE_CHAR* CustomName) : PrimaryType(Type), CustomName(CustomName)
	{
	}
};

struct HazeDefineVariable
{
	HazeDefineType Type;		//��������
	HAZE_STRING Name;			//������

	HazeDefineVariable() {}
	HazeDefineVariable(const HazeDefineType& Type, const HAZE_STRING& Name) : Type(Type), Name(Name) {}
};

struct HazeClassData
{
	std::vector<HazeDefineVariable> Vector_Data;
};