#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerPointerValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerPointerValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count);

	virtual ~HazeCompilerPointerValue() override;
};