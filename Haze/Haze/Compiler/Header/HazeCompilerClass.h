#pragma once

#include "Haze.h"

class HazeCompilerClass
{
public:
	HazeCompilerClass(const HAZE_STRING& Name, const HazeClassData& ClassData);
	~HazeCompilerClass();

private:
	HAZE_STRING Name;
	HazeClassData ClassData;
};
