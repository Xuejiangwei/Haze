#pragma once

#include <unordered_map>

#include "Haze.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunctionStack.h"

class HazeCompilerModule;

class HazeCompilerFunction
{
public:
	HazeCompilerFunction(HazeCompilerModule* Module);
	~HazeCompilerFunction();

	HazeCompilerValue* AddLocalVariable(HazeCompilerModule* Module);

private:
	HazeCompilerModule* Module;

	std::unordered_map<HAZE_STRING, HazeCompilerValue> MapLocalVariable;

	HazeCompilerFunctionStack StackFrame;
};
