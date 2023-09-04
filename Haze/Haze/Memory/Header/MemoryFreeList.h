#pragma once

#include "Haze.h"

static inline void*& NextObj(void* obj)
{
	return *(void**)obj;
}

class MemoryFreeList
{
public:
	friend class MemoryBlock;

	MemoryFreeList();

	~MemoryFreeList();

	void Push(void* Obj);

	void PushRange(void* Start, void* End, uint64 Size);

	void* Pop();

	void PopRange(void*& Start, void*& End, uint64 Num);

	bool Empty() const { return FreeList == nullptr; }

	void Clear();

private:
	void* FreeList;
	uint64 Length;
};