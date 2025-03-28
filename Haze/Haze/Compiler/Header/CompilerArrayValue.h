#pragma once

#include "CompilerValue.h"

class CompilerArrayValue : public CompilerValue
{
public:
	explicit CompilerArrayValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope, 
		HazeDataDesc desc, int count, x_uint64 dimension);

	virtual ~CompilerArrayValue() override;

	virtual x_uint32 GetSize() override { return GetSizeByHazeType(HazeValueType::Array); }

	x_uint64 GetArrayDimension() const { return m_ArrayDimension; }

private:
	x_uint64 m_ArrayDimension;
};
