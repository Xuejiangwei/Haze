#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerRefValue : public HazeCompilerValue
{
public:
	HazeCompilerRefValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, Share<HazeCompilerValue>& refValue);

	~HazeCompilerRefValue();

	const Share<HazeCompilerValue>& GetRefValue() const { return m_RefValue; }

private:
	Share<HazeCompilerValue> m_RefValue;
};
