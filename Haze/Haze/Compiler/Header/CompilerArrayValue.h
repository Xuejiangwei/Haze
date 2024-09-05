#pragma once

#include "CompilerValue.h"

class CompilerArrayValue : public CompilerValue
{
public:
	explicit CompilerArrayValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope, 
		HazeDataDesc desc, int count, uint64 dimension);

	virtual ~CompilerArrayValue() override;

	virtual uint32 GetSize() override { return GetSizeByHazeType(HazeValueType::Array); }

	uint64 GetArrayDimension() const { return m_ArrayDimension; }

private:
	uint64 m_ArrayDimension;
};
