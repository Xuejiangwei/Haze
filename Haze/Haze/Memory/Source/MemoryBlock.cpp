#include "MemoryBlock.h"
#include "HazeLog.h"

MemoryBlock::MemoryBlock(void* HeadAddress, uint64 BlockSize, uint64 UnitSize)
{
	BlockInfo.HeadAddress = HeadAddress;
	BlockInfo.BlockSize = BlockSize;
	BlockInfo.UnitSize = UnitSize;

	uint64 Length = BlockSize / UnitSize;
	BlockInfo.MemoryKeepSignal.resize(Length);

	char* Address = (char*)HeadAddress;
	while (Length-- > 0)
	{
		BlockInfo.FreeList.Push(Address);
		Address += UnitSize;
	}
}

MemoryBlock::~MemoryBlock()
{
}

void* MemoryBlock::GetTailAddress() const
{
	return (void*)((uint64)BlockInfo.HeadAddress + BlockInfo.BlockSize);
}

void* MemoryBlock::Alloca(uint64 Size)
{
	if (BlockInfo.FreeList.Length > 0)
	{
		return BlockInfo.FreeList.Pop();
	}

	return nullptr;
}

void MemoryBlock::SetNextBlock(std::unique_ptr<MemoryBlock>& Block)
{
	if (!BlockInfo.NextBlock)
	{
		BlockInfo.NextBlock = std::move(Block);
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("Repeat set next block !\n"));
	}
}

void MemoryBlock::CollectionMemory()
{
	BlockInfo.FreeList.Clear();
	for (size_t i = 0; i < BlockInfo.MemoryKeepSignal.size(); i++)
	{
		if (BlockInfo.MemoryKeepSignal[i] == 0)
		{
			BlockInfo.FreeList.Push((char*)BlockInfo.HeadAddress + i * BlockInfo.UnitSize);
		}
	}
}