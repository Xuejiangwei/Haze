#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerPointerPointerValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerPointerPointerValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count, int level);

	virtual ~HazeCompilerPointerPointerValue() override;

	int GetLevel() const { return m_Level; }

private:
	int m_Level;
};