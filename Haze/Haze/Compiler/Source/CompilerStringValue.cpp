#include "HazePch.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerStringValue.h"
#include "HazeLog.h"

CompilerStringValue::CompilerStringValue(CompilerModule* compilerModule, const HazeVariableType& defineType,
	/*HazeVariableScope scope,*/ HazeDataDesc desc, int count)
	: CompilerValue(compilerModule, defineType, /*scope,*/ desc, count), m_PureString(nullptr)
{
}

CompilerStringValue::~CompilerStringValue()
{
	m_PureString = nullptr;
}