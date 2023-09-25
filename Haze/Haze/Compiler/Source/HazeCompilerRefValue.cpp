#include "HazeCompilerRefValue.h"

HazeCompilerRefValue::HazeCompilerRefValue(HazeCompilerModule* m_Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, std::shared_ptr<HazeCompilerValue>& RefValue)
	: HazeCompilerValue(m_Module, DefineType, Scope, Desc, Count), RefValue(RefValue)
{
}

HazeCompilerRefValue::~HazeCompilerRefValue()
{
}