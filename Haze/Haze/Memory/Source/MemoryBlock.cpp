#include "HazePch.h"
#include "MemoryBlock.h"
#include "HazeMemory.h"
#include "HazeLog.h"

MemoryBlock::MemoryBlock(x_uint32 unitSize)
{
	Reuse(unitSize);
	MarkAllWhite();
}

MemoryBlock::~MemoryBlock()
{
	free(m_BlockInfo.Mark);
}

void MemoryBlock::MarkBlack(void* address)
{
	x_uint64 Index = ((x_uint64)address - (x_uint64)m_Memory) / m_BlockInfo.UnitSize;
	//assert(Index < m_BlockInfo.MarkCount);
	m_BlockInfo.Mark[Index] = (int)GC_State::Black;
}

void MemoryBlock::MarkWhite(void* address)
{
	x_uint64 Index = ((x_uint64)address - (x_uint64)m_Memory) / m_BlockInfo.UnitSize;
	m_BlockInfo.Mark[Index] = (int)GC_State::White;
}

void MemoryBlock::MarkAllWhite()
{
	memset(m_BlockInfo.Mark, (int)GC_State::White, m_BlockInfo.MarkCount);
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

	m_OnFreeListCount = 0;
}

void MemoryBlock::Recycle()
{
	m_BlockInfo.State = MemoryBlockState::Free;
}

void MemoryBlock::StartArrangeFragment()
{
	m_OnFreeListCount = 0;
	memset(m_BlockInfo.Mark, (int)GC_State::Black, m_BlockInfo.MarkCount);
}

void MemoryBlock::MarkOnFreeListCount(void* address)
{
	m_OnFreeListCount++;
	MarkWhite(address);
}

bool MemoryBlock::OnMarkFreeListEnd()
{
	return m_OnFreeListCount == m_BlockInfo.MarkCount;
}

MemoryBlockIteration::MemoryBlockIteration(MemoryBlock* block)
	: m_Block(block), m_Counter(0)
{
	if (m_Block)
	{
		for (; m_Counter < m_Block->m_BlockInfo.MarkCount; m_Counter++)
		{
			if (m_Block->m_BlockInfo.Mark[m_Counter] == (int)GC_State::White)
			{
				break;
			}
		}
	}
}

MemoryBlockIteration& MemoryBlockIteration::operator++()
{
	if (m_Block)
	{
		for (m_Counter++; m_Counter < m_Block->m_BlockInfo.MarkCount; m_Counter++)
		{
			if (m_Block->m_BlockInfo.Mark[m_Counter] == (int)GC_State::White)
			{
				return *this;
			}
		}
	}

	return *this;
}