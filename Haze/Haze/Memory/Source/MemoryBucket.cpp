#include "MemoryBucket.h"
#include "MemoryFreeList.h"

MemoryBucket::MemoryBucket(int ByteSize, uint32 InitSize) : ByteSize(ByteSize)
{
	FreeList = std::make_unique<MemoryFreeList>();
	Pool.resize(InitSize);
	CurrIndex = Pool.begin();
}

MemoryBucket::~MemoryBucket()
{
}

void* MemoryBucket::Alloca(int Size)
{
	if (!FreeList->Empty())
	{
		return FreeList->Pop();
	}

	if (ByteSize > 0)
	{
		Size = ByteSize;
	}

	void* Ret = nullptr;
	if (Queue_Recycle.size() > 0)
	{
		Ret = &Pool[Queue_Recycle.front()];
		Queue_Recycle.pop();
	}
	else
	{
		if (CurrIndex != Pool.end())
		{
			Ret = (CurrIndex)._Ptr + Size;
			CurrIndex += Size;
		}
	}

	return Ret;
}

void MemoryBucket::Dealloca(void* Alloc)
{
	FreeList->Push(Alloc);
}

void MemoryBucket::Release()
{
	FreeList->Clear();
}