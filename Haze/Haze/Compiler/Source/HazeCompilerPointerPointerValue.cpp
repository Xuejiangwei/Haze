#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerPointerPointerValue.h"
#include "HazeLog.h"

HazeCompilerPointerPointerValue::HazeCompilerPointerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, int m_Level)
	: HazeCompilerValue(Module, DefineType, Scope, Desc, Count), m_Level(m_Level)
{
}

HazeCompilerPointerPointerValue::~HazeCompilerPointerPointerValue()
{
}