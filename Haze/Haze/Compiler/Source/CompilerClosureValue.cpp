#include "HazePch.h"
#include "CompilerClosureValue.h"

CompilerClosureValue::CompilerClosureValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope, HazeDataDesc desc, int count, TemplateDefineTypes* params)
	: CompilerValue(compilerModule, defineType, scope, desc, count)
{
}

CompilerClosureValue::~CompilerClosureValue()
{
}
