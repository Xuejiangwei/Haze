#pragma once

#include "MemoryFreeList.h"

class MemoryBlock;

enum class MemoryBlockState : x_uint8
{
	Used,
	Free
};

struct MemoryBlockInfo
{
	MemoryBlock* Next;
	MemoryBlock* Prev;
	MemoryBlock* Block;
	MemoryBlockState State;
	x_uint32 MarkCount;
	x_uint32 UnitSize;
	x_uint8* Mark;

	MemoryBlockInfo() : Next(nullptr), Prev(nullptr), Block(nullptr), State(MemoryBlockState::Free), MarkCount(0), UnitSize(0), Mark(nullptr)
	{}
};

class MemoryBlock
{
	friend class HazeMemory;
	friend class MemoryBlockIteration;
public:

	MemoryBlock(x_uint32 unitSize);

	~MemoryBlock();

	void MarkBlack(void* address);

	void MarkWhite(void* address);

	void MarkAllWhite();

	bool IsUsed() const { return m_BlockInfo.State == MemoryBlockState::Used; }

	MemoryBlock* GetNext() { return m_BlockInfo.Next; }

	void SetNext(MemoryBlock* block) { m_BlockInfo.Next = block; if(block) block->m_BlockInfo.Prev = this; }

	bool IsInBlock(void* address);

	void Reuse(x_uint32 unitSize);

	void Recycle();

private:
	void StartArrangeFragment();

	void MarkOnFreeListCount(void* address);

	bool OnMarkFreeListEnd();

	bool IsPendingArrange() const { return m_OnFreeListCount > 0; }

	x_uint32 GetOnFreeListCount() const { return m_OnFreeListCount; }

private:
	x_uint32 m_OnFreeListCount;
	char m_Memory[PAGE_UNIT];
	MemoryBlockInfo m_BlockInfo;
};

class MemoryBlockIteration
{
public:
	MemoryBlockIteration(MemoryBlock* block);
	
	~MemoryBlockIteration() {}

	bool IsValid() const { return m_Block && m_Counter < m_Block->m_BlockInfo.MarkCount; }

	MemoryBlockIteration& operator++();

	MemoryBlockIteration& operator++(int) { return this->operator++(); }

	void* GetAddress() { return m_Block->m_Memory + m_Counter * m_Block->m_BlockInfo.UnitSize; }

private:
	MemoryBlock* m_Block;
	x_uint32 m_Counter;
};
