#pragma once

#include "CompilerValue.h"

class CompilerHashValue : public CompilerValue
{
public:
	explicit CompilerHashValue(CompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count, TemplateDefineTypes* params);

	virtual ~CompilerHashValue() override;

	const TemplateDefineType& GetKeyType() const { return m_KeyType; }

	const TemplateDefineType& GetValueType() const { return m_ValueType; }

	bool IsKeyType(Share<CompilerValue> key);
	bool IsValueType(Share<CompilerValue> value);
	bool KeyCanCVT(Share<CompilerValue> key);
private:
	TemplateDefineType m_KeyType;
	TemplateDefineType m_ValueType;
};