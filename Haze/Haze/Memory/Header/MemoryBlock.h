#pragma once

#include "Haze.h"
#include "MemoryFreeList.h"

class MemoryBlock;

struct MemoryBlockInfo
{
	void* HeadAddress;
	uint64 BlockSize;
	uint64 UnitSize;

	MemoryFreeList List;

	std::vector<char> MemoryKeepSignal;

	std::unique_ptr<MemoryBlock> NextBlock;//‘›≤ª π”√
};

class MemoryBlock
{
public:
	friend class GarbageCollection;

	MemoryBlock(void* HeadAddress, uint64 BlockSize, uint64 UnitSize);
	~MemoryBlock();

	void* GetHeadAddress() const { return BlockInfo.HeadAddress; }

	void* GetTailAddress() const;

	void* Alloca(uint64 Size);

	void SetNextBlock(std::unique_ptr<MemoryBlock>& Block);

	MemoryBlock* GetNextBlock() const { return BlockInfo.NextBlock.get(); }

	void CollectionMemory();

private:
	MemoryBlockInfo BlockInfo;
};

