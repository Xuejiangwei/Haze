#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeLog.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count)
	: HazeCompilerValue(Module, DefineType, Scope, Count)
{
}

HazeCompilerPointerValue::~HazeCompilerPointerValue()
{
}