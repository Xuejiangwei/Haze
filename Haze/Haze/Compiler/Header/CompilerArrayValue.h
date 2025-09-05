#pragma once

#include "CompilerValue.h"

class CompilerArrayValue : public CompilerValue
{
public:
	explicit CompilerArrayValue(CompilerModule* compilerModule, const HazeVariableType& defineType, /*HazeVariableScope scope, */
		HazeDataDesc desc, int count);

	virtual ~CompilerArrayValue() override;

	HazeVariableType GetElementType() const;

	x_uint64 GetArrayDimension() const { return m_ArrayDimension; }

private:
	x_uint64 m_ArrayDimension;
};
