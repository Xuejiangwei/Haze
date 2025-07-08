#pragma once

#include "CompilerValue.h"

class CompilerHashValue : public CompilerValue
{
public:
	explicit CompilerHashValue(CompilerModule* compilerModule, const HazeVariableType& defineType, HazeVariableScope scope, HazeDataDesc desc, int count);

	virtual ~CompilerHashValue() override;

	HazeVariableType GetKeyType() const { return m_KeyType; }
	HazeVariableType GetValueType() const { return m_ValueType; }

	bool IsKeyType(Share<CompilerValue> key);
	bool IsValueType(Share<CompilerValue> value);
private:
	HazeVariableType m_KeyType;
	HazeVariableType m_ValueType;
};