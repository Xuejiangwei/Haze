#include "HazeCompilerPointerFunction.h"

HazeCompilerPointerFunction::HazeCompilerPointerFunction(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, std::vector<HazeDefineType>* paramTypes)
	: HazeCompilerPointerValue(compilerModule, defineType, scope, desc, count), m_ParamTypes(std::move(*paramTypes))
{
}

HazeCompilerPointerFunction::~HazeCompilerPointerFunction()
{
}