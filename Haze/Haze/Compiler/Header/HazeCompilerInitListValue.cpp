#include "HazeCompilerInitListValue.h"

HazeCompilerInitListValue::HazeCompilerInitListValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count)
	: HazeCompilerValue(Module, DefineType, Scope, Count)
{
}

HazeCompilerInitListValue::~HazeCompilerInitListValue()
{
}
