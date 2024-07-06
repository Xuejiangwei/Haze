#include "HazePch.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeLog.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count)
	: HazeCompilerValue(compilerModule, defineType, scope, desc, count)
{
}

HazeCompilerPointerValue::~HazeCompilerPointerValue()
{
}