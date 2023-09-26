#pragma once

#include "Haze.h"
#include "HazeCompilerValue.h"

class HazeCompilerEnum
{
public:
	HazeCompilerEnum();

	~HazeCompilerEnum();

private:
	std::vector<std::shared_ptr<HazeCompilerValue>> m_EnumValues;
};

