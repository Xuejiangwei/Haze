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
	HazeValueType PrimaryType;								//Type类型
	

	//待优化
	HazeValueType PointerToType;				//指针指向类型
	HAZE_STRING CustomName;				//自定义类型名

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
	HazeDefineType Type;		//变量类型
	HAZE_STRING Name;			//变量名

	HazeDefineVariable() {}
	HazeDefineVariable(const HazeDefineType& Type, const HAZE_STRING& Name) : Type(Type), Name(Name) {}
};

struct HazeLocalVariable
{
	HazeDefineVariable Variable;
	int Offset;
};

struct HazeClassData
{
	std::vector<HazeDefineVariable> Vector_Data;
};