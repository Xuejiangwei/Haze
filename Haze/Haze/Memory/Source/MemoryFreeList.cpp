#include "MemoryFreeList.h"

MemoryFreeList::MemoryFreeList() : FreeList(nullptr), Length(0)
{
}

MemoryFreeList::~MemoryFreeList()
{
}

void MemoryFreeList::Push(void* Obj)
{
	NextObj(Obj) = FreeList;
	FreeList = Obj;
	++Length;
}

void MemoryFreeList::PushRange(void* Start, void* End, uint64 Size)
{
	NextObj(End) = FreeList;
	FreeList = Start;
	Length += Size;
}

void* MemoryFreeList::Pop()
{
	void* obj = FreeList;
	FreeList = NextObj(obj);
	--Length;
	return obj;
}

void MemoryFreeList::PopRange(void*& Start, void*& End, uint64 Num)
{
	Start = FreeList;
	End = Start;
	for (size_t i = 0; i < Num - 1; i++)
	{
		End = NextObj(End);
	}
	FreeList = NextObj(End);
	Length -= Num;
	NextObj(End) = nullptr;
}

void MemoryFreeList::Clear()
{
	FreeList = nullptr;
	Length = 0;
}