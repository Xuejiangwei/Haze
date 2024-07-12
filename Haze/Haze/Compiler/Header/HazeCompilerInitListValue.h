#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerInitListValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerInitListValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, 
		HazeVariableScope scope, HazeDataDesc desc, int count);
	
	~HazeCompilerInitListValue();

	void ResetInitializeList(V_Array<Share<HazeCompilerValue>>& freeList) { m_InitList = Move(freeList); }

	const V_Array<Share<HazeCompilerValue>>& GetList() const { return m_InitList; }

private:
	V_Array<Share<HazeCompilerValue>> m_InitList;
};
