#include "HazeCompilerInitListValue.h"

HazeCompilerInitListValue::HazeCompilerInitListValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count)
	: HazeCompilerValue(Module, DefineType, Scope, Desc, Count)
{
}

HazeCompilerInitListValue::~HazeCompilerInitListValue()
{
}
