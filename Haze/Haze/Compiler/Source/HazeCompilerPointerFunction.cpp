#include "HazeCompilerPointerFunction.h"

HazeCompilerPointerFunction::HazeCompilerPointerFunction(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, std::vector<HazeDefineType> Vector_ParamType)
	: HazeCompilerPointerValue(Module, DefineType, Scope, Count), Vector_ParamType(Vector_ParamType)
{
}

HazeCompilerPointerFunction::~HazeCompilerPointerFunction()
{
}
