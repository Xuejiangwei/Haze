#include "HazePch.h"
#include "CompilerArrayValue.h"
#include "CompilerClass.h"
#include "CompilerModule.h"
#include "CompilerFunction.h"
#include "CompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerClassValue.h"
#include "HazeLogDefine.h"

CompilerArrayValue::CompilerArrayValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
	HazeDataDesc desc, int count, uint64 dimension)
	: CompilerValue(compilerModule, defineType, scope, desc, count), m_ArrayDimension(dimension)
{
}

CompilerArrayValue::~CompilerArrayValue()
{
}