#include "HazePch.h"
#include "HazeCompilerInitListValue.h"

HazeCompilerInitListValue::HazeCompilerInitListValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, 
	HazeVariableScope scope, HazeDataDesc desc, int count)
	: HazeCompilerValue(compilerModule, defineType, scope, desc, count)
{
}

HazeCompilerInitListValue::~HazeCompilerInitListValue()
{
}