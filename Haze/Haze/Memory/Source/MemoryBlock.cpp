#include "HazePch.h"
#include "MemoryBlock.h"
#include "HazeMemory.h"
#include "HazeLog.h"

MemoryBlock::MemoryBlock(x_uint32 unitSize)
{
	Reuse(unitSize);
}

MemoryBlock::~MemoryBlock()
{
	free(m_BlockInfo.Mark);
}

void MemoryBlock::SetAllWhite()
{
	for (x_uint32 i = 0; i < m_BlockInfo.MarkCount; i++)
	{
		m_BlockInfo.Mark[0] = (int)GC_State::White;
	}
}

void MemoryBlock::MarkBlack(void* address)
{
	x_uint64 Index = ((x_uint64)address - (x_uint64)m_Memory) / m_BlockInfo.UnitSize;
	//assert(Index < m_BlockInfo.MarkCount);
	m_BlockInfo.Mark[Index] = (int)GC_State::Black;
}

void MemoryBlock::MarkWrite(void* address)
{
	x_uint64 Index = ((x_uint64)address - (x_uint64)m_Memory) / m_BlockInfo.UnitSize;
	m_BlockInfo.Mark[Index] = (int)GC_State::White;
}

bool MemoryBlock::IsInBlock(void* address)
{
	return m_Memory <= address && address < &m_Memory + 1;
}

void MemoryBlock::Reuse(x_uint32 unitSize)
{
	if (unitSize != m_BlockInfo.UnitSize && m_BlockInfo.Mark)
	{
		free(m_BlockInfo.Mark);
	}

	m_BlockInfo.MarkCount = _countof(m_Memory) / unitSize;
	m_BlockInfo.Mark = (x_uint8*)malloc(sizeof(*m_BlockInfo.Mark) * m_BlockInfo.MarkCount);
	m_BlockInfo.State = MemoryBlockState::Used;
	m_BlockInfo.UnitSize = unitSize;
}

void MemoryBlock::Recycle()
{
	m_BlockInfo.State = MemoryBlockState::Free;
}
