#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerRefValue : public HazeCompilerValue
{
public:
	HazeCompilerRefValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, std::shared_ptr<HazeCompilerValue>& refValue);

	~HazeCompilerRefValue();

	const std::shared_ptr<HazeCompilerValue>& GetRefValue() const { return m_RefValue; }

private:
	std::shared_ptr<HazeCompilerValue> m_RefValue;
};
