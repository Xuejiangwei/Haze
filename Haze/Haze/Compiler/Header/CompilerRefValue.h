#pragma once

#include "CompilerValue.h"

class CompilerRefValue : public CompilerValue
{
public:
	CompilerRefValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, Share<CompilerValue>& refValue);

	~CompilerRefValue();

	const Share<CompilerValue>& GetRefValue() const { return m_RefValue; }

private:
	Share<CompilerValue> m_RefValue;
};
