#pragma once

#include "HazeCompilerPointerValue.h"

class HazeCompilerPointerArray : public HazeCompilerPointerValue
{
public:
	explicit HazeCompilerPointerArray(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize);

	virtual ~HazeCompilerPointerArray() override;

	uint32 GetSizeByLevel(uint32 Level);

private:
	std::vector<std::shared_ptr<HazeCompilerValue>> Vector_ArraySize;
};
