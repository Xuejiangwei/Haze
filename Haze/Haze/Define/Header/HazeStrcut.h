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
	HazeValueType Type;			//Token����
	HAZE_STRING CustomName;		//�Զ���������

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
	HazeDefineData Type;		//��������
	HAZE_STRING Name;			//������

	HazeDefineVariable() {}
	HazeDefineVariable(const HazeDefineData& Type, const HAZE_STRING& Name) : Type(Type), Name(Name) {}
};

struct HazeClassData
{
	std::vector<HazeDefineVariable> Vector_Data;
};