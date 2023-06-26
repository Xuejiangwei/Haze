#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerInitListValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerInitListValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count);
	~HazeCompilerInitListValue();

	void ResetInitializeList(std::vector<std::shared_ptr<HazeCompilerValue>>& List) { Vector_InitList = std::move(List); }

	const std::vector<std::shared_ptr<HazeCompilerValue>>& GetList() const { return Vector_InitList; }

private:
	std::vector<std::shared_ptr<HazeCompilerValue>> Vector_InitList;
};

