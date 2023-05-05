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
	void* TailAddress = GetTailAddress();
	while (Length-- > 0)
	{
		BlockInfo.List.Push(Address);
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
	if (BlockInfo.List.Length > 0)
	{
		return BlockInfo.List.Pop();
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
	BlockInfo.List.Clear();
	for (size_t i = 0; i < BlockInfo.MemoryKeepSignal.size(); i++)
	{
		if (BlockInfo.MemoryKeepSignal[i] == 0)
		{
			BlockInfo.List.Push((char*)BlockInfo.HeadAddress + i * BlockInfo.UnitSize);
		}
	}
}
