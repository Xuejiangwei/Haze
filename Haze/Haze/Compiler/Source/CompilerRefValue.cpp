#include "HazePch.h"
#include "CompilerRefValue.h"

CompilerRefValue::CompilerRefValue(CompilerModule* compilerModule, const HazeVariableType& defineType,
	/*HazeVariableScope scope,*/ HazeDataDesc desc, int count, Share<CompilerValue>& refValue)
	: CompilerValue(compilerModule, defineType, /*scope,*/ desc, count), m_RefValue(refValue)
{
}

CompilerRefValue::~CompilerRefValue()
{
}