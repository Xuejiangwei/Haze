#pragma once

#include "CompilerValue.h"

class CompilerObjectBaseValue : public CompilerValue
{
public:
	explicit CompilerObjectBaseValue(CompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count);

	virtual ~CompilerObjectBaseValue() override;

private:
};
