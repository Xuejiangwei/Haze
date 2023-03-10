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

	HazeDefineData() : Type(HazeValueType::Null)
	{
	}

	HazeDefineData(HazeValueType Type, HAZE_STRING CustomName) : Type(Type), CustomName(std::move(CustomName))
	{
	}
};

struct HazeDefineVariable
{
	HazeDefineData Type;		//��������
	HAZE_STRING Name;			//������
};