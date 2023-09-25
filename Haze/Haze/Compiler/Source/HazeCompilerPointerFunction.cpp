#include "HazeCompilerPointerFunction.h"

HazeCompilerPointerFunction::HazeCompilerPointerFunction(HazeCompilerModule* m_Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, std::vector<HazeDefineType> Vector_ParamType)
	: HazeCompilerPointerValue(m_Module, DefineType, Scope, Desc, Count), Vector_ParamType(Vector_ParamType)
{
}

HazeCompilerPointerFunction::~HazeCompilerPointerFunction()
{
}