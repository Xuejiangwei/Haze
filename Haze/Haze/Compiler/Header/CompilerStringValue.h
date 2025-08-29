#pragma once

#include "CompilerValue.h"

class CompilerStringValue : public CompilerValue
{
public:
	explicit CompilerStringValue(CompilerModule* compilerModule, const HazeVariableType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count);

	virtual ~CompilerStringValue() override;

	void SetPureString(const STDString* str) { m_PureString = str; }

	const STDString* GetPureString() const { return m_PureString; }

private:
	const STDString* m_PureString;
};