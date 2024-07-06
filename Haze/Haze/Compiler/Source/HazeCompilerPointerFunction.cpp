#include "HazePch.h"
#include "HazeCompilerPointerFunction.h"

HazeCompilerPointerFunction::HazeCompilerPointerFunction(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, V_Array<HazeDefineType>* paramTypes)
	: HazeCompilerPointerValue(compilerModule, defineType, scope, desc, count)
{
	if (paramTypes)
	{
		m_ParamTypes = std::move(*paramTypes);
	}
}

HazeCompilerPointerFunction::~HazeCompilerPointerFunction()
{
}