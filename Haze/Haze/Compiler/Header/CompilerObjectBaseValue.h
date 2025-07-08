#pragma once

#include "CompilerValue.h"

class CompilerObjectBaseValue : public CompilerValue
{
public:
	explicit CompilerObjectBaseValue(CompilerModule* compilerModule, const HazeVariableType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count);

	virtual ~CompilerObjectBaseValue() override;

	HazeValueType GetHazeBaseType() const { return m_HazeBaseType; }

private:
	HazeValueType m_HazeBaseType;
};
