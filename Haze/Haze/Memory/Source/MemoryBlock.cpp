#include "MemoryBlock.h"
#include "HazeMemory.h"
#include "HazeLog.h"

MemoryBlock::MemoryBlock(uint32 unitSize)
{
	Reuse(unitSize);
}

MemoryBlock::~MemoryBlock()
{
	free(m_BlockInfo.Mark);
}

void MemoryBlock::SetAllWhite()
{
	for (uint32 i = 0; i < m_BlockInfo.MarkCount; i++)
	{
		m_BlockInfo.Mark[0] = (int)GC_State::White;
	}
}

void MemoryBlock::MarkBlack(void* address)
{
	uint64 Index = ((uint64)address - (uint64)m_Memory) / m_BlockInfo.UnitSize;
	assert(Index < m_BlockInfo.MarkCount);
	m_BlockInfo.Mark[Index] = (int)GC_State::Black;
}

bool MemoryBlock::IsInBlock(void* address)
{
	return m_Memory <= address && address < &m_Memory + 1;
}

void MemoryBlock::Reuse(uint32 unitSize)
{
	if (unitSize != m_BlockInfo.UnitSize && m_BlockInfo.Mark)
	{
		free(m_BlockInfo.Mark);
	}

	m_BlockInfo.MarkCount = _countof(m_Memory) / unitSize;
	m_BlockInfo.Mark = (uint8*)malloc(sizeof(*m_BlockInfo.Mark) * m_BlockInfo.MarkCount);
	m_BlockInfo.State = MemoryBlockState::Used;
	m_BlockInfo.UnitSize = unitSize;
}

void MemoryBlock::Recycle()
{
	m_BlockInfo.State = MemoryBlockState::Free;
}
