#pragma once

#include "CompilerValue.h"

class CompilerStringValue : public CompilerValue
{
public:
	explicit CompilerStringValue(CompilerModule* compilerModule, const HazeVariableType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count);

	virtual ~CompilerStringValue() override;

	void SetPureString(const HString* str) { m_PureString = str; }

	const HString* GetPureString() const { return m_PureString; }

private:
	const HString* m_PureString;
};