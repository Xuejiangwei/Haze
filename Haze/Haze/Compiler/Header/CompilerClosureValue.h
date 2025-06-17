#pragma once

#include "CompilerValue.h"

class CompilerClosureValue : public CompilerValue
{
public:
	explicit CompilerClosureValue(CompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count, TemplateDefineTypes* params);

	virtual ~CompilerClosureValue() override;

	void SetPureString(const HString* str) { m_PureString = str; }

	const HString* GetPureString() const { return m_PureString; }

private:
	const HString* m_PureString;
};