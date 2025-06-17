#include "HazePch.h"
#include "CompilerObjectBaseValue.h"

CompilerObjectBaseValue::CompilerObjectBaseValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope, HazeDataDesc desc, int count)
	: CompilerValue(compilerModule, defineType, scope, desc, count)
{
}

CompilerObjectBaseValue::~CompilerObjectBaseValue()
{
}
