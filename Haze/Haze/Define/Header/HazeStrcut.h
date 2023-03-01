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
	HazeValueType Type;			//Token����
	HAZE_STRING CustomName;		//�Զ���������
};

struct HazeDefineVariable
{
	HazeDefineData Type;		//��������
	HAZE_STRING Name;			//������
};