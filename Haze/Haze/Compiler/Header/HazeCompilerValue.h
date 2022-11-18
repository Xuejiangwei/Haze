#pragma once

#include "Haze.h"

class HazeCompilerModule;

class HazeCompilerValue
{
public:
	HazeCompilerValue(HazeCompilerModule* Module);
	~HazeCompilerValue();

	void StoreValue(HazeCompilerValue* Value);

private:
	HazeValue Value;
	HazeCompilerModule* Module;
};
