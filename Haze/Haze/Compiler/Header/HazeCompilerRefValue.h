#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerRefValue : public HazeCompilerValue
{
public:
	HazeCompilerRefValue(HazeCompilerModule* m_Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, std::shared_ptr<HazeCompilerValue>& RefValue);
	~HazeCompilerRefValue();

	const std::shared_ptr<HazeCompilerValue>& GetRefValue() const { return RefValue; }

private:
	std::shared_ptr<HazeCompilerValue> RefValue;
};
