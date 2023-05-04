#pragma once

#include "Haze.h"

static inline void*& NextObj(void* obj)
{
	return *(void**)obj;
}

class MemoryFreeList
{
public:
	void Push(void* Obj);
	
	void PushRange(void* Start, void* End, size_t size);
	
	void* Pop();

	void PopRange(void*& Start, void*& End, size_t n);

	bool Empty() const { return List == nullptr; }

	void Clear();

private:
	void* List;
	uint64 Length;
};