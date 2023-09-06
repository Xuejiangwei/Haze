#include "MemoryBlock.h"
#include "HazeMemory.h"
#include "HazeLog.h"

MemoryBlock::MemoryBlock(uint32 unitSize)
{
	Reuse(unitSize);
}

MemoryBlock::~MemoryBlock()
{
	free(BlockInfo.Mark);
}

void MemoryBlock::SetAllWhite()
{
	for (int i = 0; i < BlockInfo.MarkCount; i++)
	{
		BlockInfo.Mark[0] = (int)GC_State::White;
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
	return m_Memory <= address && address < &m_Memory + 1;
}

void MemoryBlock::Reuse(uint32 unitSize)
{
	if (unitSize != BlockInfo.UnitSize && BlockInfo.Mark)
	{
		free(BlockInfo.Mark);
	}

	BlockInfo.MarkCount = _countof(m_Memory) / unitSize;
	BlockInfo.Mark = (uint8*)malloc(sizeof(*BlockInfo.Mark) * BlockInfo.MarkCount);
	BlockInfo.State = MemoryBlockState::Used;
	BlockInfo.UnitSize = unitSize;
}

void MemoryBlock::Recycle()
{
	BlockInfo.State = MemoryBlockState::Free;
}
