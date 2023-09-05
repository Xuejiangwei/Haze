#pragma once

#include "Haze.h"
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
	int Mark[4096];
};

class MemoryBlock
{
public:
	friend class HazeMemory;

	MemoryBlock(uint64 UnitSize);

	~MemoryBlock();

	void SetAllWhite();

	void MarkBlack(void* address);

	bool IsUsed() const { return BlockInfo.State == MemoryBlockState::Used; }

	MemoryBlock* GetNext() { return BlockInfo.Next; }

	void SetNext(MemoryBlock* block) { BlockInfo.Next = block; block->BlockInfo.Prev = this; }

	bool IsInBlock(void* address);

private:
	char m_Memory[4096];
	MemoryBlockInfo BlockInfo;
};
