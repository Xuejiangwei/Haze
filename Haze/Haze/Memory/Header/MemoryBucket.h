#pragma once

#include "Haze.h"
#include <queue>

class MemoryFreeList;

class MemoryBucket
{
public:
	friend class MemoryPool;

	MemoryBucket(int ByteSize, uint32 InitSize);
	~MemoryBucket();

private:
	void* Alloca(int Size = 0);

	void Dealloca(void* Alloc);

	void Release();

private:
	std::queue<uint32> Queue_Recycle;
	std::vector<char> Pool;

	std::vector<char>::iterator CurrIndex;
	int ByteSize;

	std::unique_ptr<MemoryFreeList> FreeList;
};