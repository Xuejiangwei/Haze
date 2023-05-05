#pragma once

#include "Haze.h"

class MemoryBlock;
class MemoryPage;

struct MemoryPageInfo
{
	std::unique_ptr<MemoryBlock> HeadBlock;

	uint32 PageByteSize;
	uint32 UnitSize;
	
	std::unique_ptr<MemoryPage> NextPage;
};

class MemoryPage
{
public:
	friend class GarbageCollection;

	MemoryPage(uint32 PageByteSize, uint32 BlockUnitSize);
	~MemoryPage();

	void* TryAlloca(uint64 Size);

	MemoryBlock* GetLastBlock();

	void SetNextPage(std::unique_ptr<MemoryPage> Page);

	bool IsInPage(void* Address);

private:
	MemoryPageInfo PageInfo;
};

