#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerPointerPointerValue.h"
#include "HazeLog.h"

HazeCompilerPointerPointerValue::HazeCompilerPointerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, int Level)
	: HazeCompilerValue(Module, DefineType, Scope, Count), Level(Level)
{

}

HazeCompilerPointerPointerValue::~HazeCompilerPointerPointerValue()
{
}