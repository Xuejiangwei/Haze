#include "MemoryBlock.h"
#include "HazeMemory.h"
#include "HazeLog.h"

MemoryBlock::MemoryBlock(uint64 UnitSize)
{
	BlockInfo.UnitSize = UnitSize;
}

MemoryBlock::~MemoryBlock()
{
}

void MemoryBlock::SetAllWhite()
{
	for (auto& mark : BlockInfo.Mark)
	{
		mark = (int)GC_State::White;
	}
}

void MemoryBlock::MarkBlack(void* address)
{
	uint64 Index = ((uint64)address - (uint64)m_Memory) / BlockInfo.UnitSize;
	assert(Index < BlockInfo.MarkCount);
	BlockInfo.Mark[Index] = (int)GC_State::Black;
}

bool MemoryBlock::IsInBlock(void* address)
{
	return m_Memory <= address && address < m_Memory + 1;
}
