#include "HazePch.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerArrayValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeLog.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(CompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count)
	: CompilerValue(compilerModule, defineType, scope, desc, count)
{
}

HazeCompilerPointerValue::~HazeCompilerPointerValue()
{
}