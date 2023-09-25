#include "HazeCompilerInitListValue.h"

HazeCompilerInitListValue::HazeCompilerInitListValue(HazeCompilerModule* m_Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count)
	: HazeCompilerValue(m_Module, DefineType, Scope, Desc, Count)
{
}

HazeCompilerInitListValue::~HazeCompilerInitListValue()
{
}