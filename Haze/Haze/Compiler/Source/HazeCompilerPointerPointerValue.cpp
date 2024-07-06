#include "HazePch.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerPointerPointerValue.h"
#include "HazeLog.h"

HazeCompilerPointerPointerValue::HazeCompilerPointerPointerValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, int level)
	: HazeCompilerValue(compilerModule, defineType, scope, desc, count), m_Level(level)
{
}

HazeCompilerPointerPointerValue::~HazeCompilerPointerPointerValue()
{
}