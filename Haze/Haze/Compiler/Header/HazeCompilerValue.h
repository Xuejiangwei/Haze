#pragma once

#include "Haze.h"

class HazeCompilerModule;

class HazeCompilerValue
{
public:
	HazeCompilerValue() {}

	HazeCompilerValue(HazeCompilerModule* Module);

	HazeCompilerValue(HazeCompilerModule* Module, HazeValueType Type);

	HazeCompilerValue(HazeCompilerModule* Module, HazeValue Value);


	~HazeCompilerValue();

	void StoreValue(HazeCompilerValue* Value);

private:
	HazeValue Value;
	HazeCompilerModule* Module;
};
