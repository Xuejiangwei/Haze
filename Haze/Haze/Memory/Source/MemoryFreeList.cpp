#include "HazePch.h"
#include "MemoryFreeList.h"

MemoryFreeList::MemoryFreeList() : m_FreeList(nullptr), m_Length(0)
{
}

MemoryFreeList::~MemoryFreeList()
{
}

void MemoryFreeList::Push(void* obj)
{
	NextObj(obj) = m_FreeList;
	m_FreeList = obj;
	++m_Length;
}

void MemoryFreeList::PushRange(void* start, void* end, uint64 size)
{
	NextObj(end) = m_FreeList;
	m_FreeList = start;
	m_Length += size;
}

void* MemoryFreeList::Pop()
{
	void* obj = m_FreeList;
	m_FreeList = NextObj(obj);
	--m_Length;
	return obj;
}

void MemoryFreeList::PopRange(void*& start, void*& end, uint64 num)
{
	start = m_FreeList;
	end = start;
	for (size_t i = 0; i < num - 1; i++)
	{
		end = NextObj(end);
	}
	m_FreeList = NextObj(end);
	m_Length -= num;
	NextObj(end) = nullptr;
}

void MemoryFreeList::Clear()
{
	m_FreeList = nullptr;
	m_Length = 0;
}