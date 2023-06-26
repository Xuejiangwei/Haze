#include "HazeCompilerRefValue.h"

HazeCompilerRefValue::HazeCompilerRefValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, std::shared_ptr<HazeCompilerValue>& RefValue)
	: HazeCompilerValue(Module, DefineType, Scope, Desc, Count), RefValue(RefValue)
{
}

HazeCompilerRefValue::~HazeCompilerRefValue()
{
}