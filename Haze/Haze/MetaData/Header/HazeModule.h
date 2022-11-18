#pragma once

#include <unordered_map>

#include "Haze.h"

class HazeValue;
class HazeClass;
class HazeFunction;

class HazeModule
{
public:
	HazeModule();
	~HazeModule();

	HazeValue* AddGlobalVariable();

private:
	std::unordered_map<HAZE_STRING, HazeCompilerValue> MapGlobalVariables;
	std::vector<HazeClass> Classes;
	std::vector<HazeFunction> Functions;
};
