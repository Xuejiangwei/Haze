#pragma once

#include <queue>

class MemoryBucket
{
public:
	friend class MemoryPool;

	MemoryBucket(unsigned int ByteSize, unsigned int Size);
	~MemoryBucket();

private:
	void* Alloca(int Size = 0);

	void Release();

private:
	std::queue<unsigned int> Queue_Recycle;
	std::vector<char> Pool;

	std::vector<char>::iterator CurrIndex;
	unsigned int ByteSize;
};