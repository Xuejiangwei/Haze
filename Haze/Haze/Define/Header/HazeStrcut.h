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

struct HazeDefineData
{
	HazeValueType Type;			//Token类型
	HAZE_STRING CustomName;		//自定义类型名

	HazeDefineData() : Type(HazeValueType::Void)
	{
	}

	HazeDefineData(HazeValueType Type, const HAZE_STRING& CustomName) : Type(Type), CustomName(CustomName)
	{
	}

	HazeDefineData(HazeValueType Type, const HAZE_CHAR* CustomName) : Type(Type), CustomName(CustomName)
	{
	}
};

struct HazeDefineVariable
{
	HazeDefineData Type;		//变量类型
	HAZE_STRING Name;			//变量名

	HazeDefineVariable() {}
	HazeDefineVariable(const HazeDefineData& Type, const HAZE_STRING& Name) : Type(Type), Name(Name) {}
};

struct HazeClassData
{
	std::vector<HazeDefineVariable> Vector_Data;
};