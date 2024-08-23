#pragma once

#include "CompilerValue.h"

class CompilerStringValue : public CompilerValue
{
public:
	explicit CompilerStringValue(CompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count);

	virtual ~CompilerStringValue() override;

private:
	
};