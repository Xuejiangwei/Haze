#include "HazeCompilerRefValue.h"

HazeCompilerRefValue::HazeCompilerRefValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, std::shared_ptr<HazeCompilerValue>& refValue)
	: HazeCompilerValue(compilerModule, defineType, scope, desc, count), m_RefValue(refValue)
{
}

HazeCompilerRefValue::~HazeCompilerRefValue()
{
}