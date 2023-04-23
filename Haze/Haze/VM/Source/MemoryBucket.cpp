#include "MemoryBucket.h"


MemoryBucket::MemoryBucket(unsigned int ByteSize, unsigned int Size) : ByteSize(ByteSize)
{
	Pool.resize(Size);
	CurrIndex = Pool.begin();
}

MemoryBucket::~MemoryBucket()
{
}

void* MemoryBucket::Alloca(int Size)
{
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
		else
		{
			Ret = &Pool.back();
			Pool.resize(Pool.size() + Size);
		}
	}
	return Ret;
}

void MemoryBucket::Release()
{

}
