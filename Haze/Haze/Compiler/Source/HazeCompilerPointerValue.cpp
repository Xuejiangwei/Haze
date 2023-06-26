#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeLog.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count)
	: HazeCompilerValue(Module, DefineType, Scope, Desc, Count)
{
}

HazeCompilerPointerValue::~HazeCompilerPointerValue()
{
}