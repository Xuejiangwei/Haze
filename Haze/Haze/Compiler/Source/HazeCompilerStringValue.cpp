#include "HazePch.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerStringValue.h"
#include "HazeLog.h"

HazeCompilerStringValue::HazeCompilerStringValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count)
	: HazeCompilerValue(compilerModule, defineType, scope, desc, count)
{
}

HazeCompilerStringValue::~HazeCompilerStringValue()
{
}