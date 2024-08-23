#include "HazePch.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerStringValue.h"
#include "HazeLog.h"

CompilerStringValue::CompilerStringValue(CompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count)
	: CompilerValue(compilerModule, defineType, scope, desc, count)
{
}

CompilerStringValue::~CompilerStringValue()
{
}