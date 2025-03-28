#pragma once

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

	void Push(void* obj);

	void PushRange(void* start, void* end, x_uint64 size);

	void* Pop();

	void PopRange(void*& start, void*& end, x_uint64 num);

	bool Empty() const { return m_FreeList == nullptr; }

	void Clear();

	bool HasMemory() { return m_Length > 0; }

private:
	void* m_FreeList;
	x_uint64 m_Length;
};