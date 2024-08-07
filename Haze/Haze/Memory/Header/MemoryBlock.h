#pragma once

#include "MemoryFreeList.h"

class MemoryBlock;

enum class MemoryBlockState : uint8
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
	uint32 MarkCount;
	uint32 UnitSize;
	uint8* Mark;

	MemoryBlockInfo() : Next(nullptr), Prev(nullptr), Block(nullptr), State(MemoryBlockState::Free), MarkCount(0), UnitSize(0), Mark(nullptr)
	{}
};

class MemoryBlock
{
public:
	friend class HazeMemory;

	MemoryBlock(uint32 unitSize);

	~MemoryBlock();

	void SetAllWhite();

	void MarkBlack(void* address);

	void MarkWrite(void* address);

	bool IsUsed() const { return m_BlockInfo.State == MemoryBlockState::Used; }

	MemoryBlock* GetNext() { return m_BlockInfo.Next; }

	void SetNext(MemoryBlock* block) { m_BlockInfo.Next = block; block->m_BlockInfo.Prev = this; }

	bool IsInBlock(void* address);

	void Reuse(uint32 unitSize);

	void Recycle();

private:
	char m_Memory[4096];
	MemoryBlockInfo m_BlockInfo;
};
