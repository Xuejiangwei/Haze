#pragma once

#include "CompilerValue.h"

class HazeCompilerPointerValue : public CompilerValue
{
public:
	explicit HazeCompilerPointerValue(CompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count);

	virtual ~HazeCompilerPointerValue() override;
};