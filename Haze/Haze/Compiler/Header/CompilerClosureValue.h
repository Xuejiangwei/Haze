#pragma once

#include "CompilerPointerFunction.h"

class CompilerClosureValue : public CompilerPointerFunction
{
public:
	explicit CompilerClosureValue(CompilerModule* compilerModule, const HazeVariableType& defineType, /*HazeVariableScope scope,*/
		HazeDataDesc desc, int count);

	virtual ~CompilerClosureValue() override;
};