#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerPointerPointerValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerPointerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, int Level);

	virtual ~HazeCompilerPointerPointerValue() override;

	int GetLevel() const { return Level; }

private:
	int Level;
};