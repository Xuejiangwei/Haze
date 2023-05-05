#include "MemoryFreeList.h"

MemoryFreeList::MemoryFreeList() : List(nullptr), Length(0)
{
}

MemoryFreeList::~MemoryFreeList()
{
}

void MemoryFreeList::Push(void* Obj)
{
	NextObj(Obj) = List;
	List = Obj;
	++Length;
}

void MemoryFreeList::PushRange(void* Start, void* End, uint64 Size)
{
	NextObj(End) = List;
	List = Start;
	Length += Size;
}

void* MemoryFreeList::Pop()
{
	void* obj = List;
	List = NextObj(obj);
	--Length;
	return obj;
}

void MemoryFreeList::PopRange(void*& Start, void*& End, uint64 Num)
{
	Start = List;
	End = Start;
	for (size_t i = 0; i < Num - 1; i++)
	{
		End = NextObj(End);
	}
	List = NextObj(End);
	Length -= Num;
	NextObj(End) = nullptr;
}

void MemoryFreeList::Clear()
{
	List = nullptr;
	Length = 0;
}
