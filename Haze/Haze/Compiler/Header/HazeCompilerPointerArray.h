#pragma once

#include "HazeCompilerPointerValue.h"

class HazeCompilerPointerArray : public HazeCompilerPointerValue
{
public:
	explicit HazeCompilerPointerArray(HazeCompilerModule* m_Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize);

	virtual ~HazeCompilerPointerArray() override;

	uint32 GetSizeByLevel(uint32 m_Level);

	const std::vector<std::shared_ptr<HazeCompilerValue>>& GetArraySize() const { return Vector_ArraySize; }

private:
	std::vector<std::shared_ptr<HazeCompilerValue>> Vector_ArraySize;
};
