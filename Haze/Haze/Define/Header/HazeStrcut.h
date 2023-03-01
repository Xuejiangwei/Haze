#pragma once

#include "HazeToken.h"
#include "HazeValue.h"

enum class HazeSectionSignal : unsigned __int8
{
	Global,
	Function,
	Class,
};

struct HazeDefineData
{
	HazeValueType Type;			//Token类型
	HAZE_STRING CustomName;		//自定义类型名
};

struct HazeDefineVariable
{
	HazeDefineData Type;		//变量类型
	HAZE_STRING Name;			//变量名
};