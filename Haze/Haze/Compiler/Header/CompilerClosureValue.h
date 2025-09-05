#pragma once

#include "CompilerPointerFunction.h"

class CompilerClosureValue : public CompilerPointerFunction
{
public:
	explicit CompilerClosureValue(CompilerModule* compilerModule, const HazeVariableType& defineType, /*HazeVariableScope scope,*/
		HazeDataDesc desc, int count, TemplateDefineTypes* params);

	virtual ~CompilerClosureValue() override;
};