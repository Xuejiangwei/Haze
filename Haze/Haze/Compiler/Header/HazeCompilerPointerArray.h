#pragma once

#include "HazeCompilerPointerValue.h"

class HazeCompilerPointerArray : public HazeCompilerPointerValue
{
public:
	explicit HazeCompilerPointerArray(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, std::vector<std::shared_ptr<HazeCompilerValue>> arraySize);

	virtual ~HazeCompilerPointerArray() override;

	uint32 GetSizeByLevel(uint32 level);

	const std::vector<std::shared_ptr<HazeCompilerValue>>& GetArraySize() const { return m_ArraySize; }

private:
	std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize;
};
