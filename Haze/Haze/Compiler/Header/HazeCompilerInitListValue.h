#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerInitListValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerInitListValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, 
		HazeVariableScope scope, HazeDataDesc desc, int count);
	
	~HazeCompilerInitListValue();

	void ResetInitializeList(std::vector<std::shared_ptr<HazeCompilerValue>>& freeList) { m_InitList = std::move(freeList); }

	const std::vector<std::shared_ptr<HazeCompilerValue>>& GetList() const { return m_InitList; }

private:
	std::vector<std::shared_ptr<HazeCompilerValue>> m_InitList;
};
