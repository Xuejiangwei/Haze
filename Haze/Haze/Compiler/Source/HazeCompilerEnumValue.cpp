#include "HazeCompilerEnumValue.h"
#include "HazeCompilerEnum.h"

HazeCompilerEnumValue::HazeCompilerEnumValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, 
	HazeVariableScope scope, HazeDataDesc desc, int count)
	: HazeCompilerValue(compilerModule, defineType, scope, desc, count)
{
}

HazeCompilerEnumValue::~HazeCompilerEnumValue()
{
}
