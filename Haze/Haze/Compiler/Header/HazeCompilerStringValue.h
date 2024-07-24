#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerStringValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerStringValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count);

	virtual ~HazeCompilerStringValue() override;

private:
	
};