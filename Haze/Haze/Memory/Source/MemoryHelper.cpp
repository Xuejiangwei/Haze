#include "HazePch.h"
#include "MemoryHelper.h"
#include "MemoryDefine.h"

static x_int64 _RoundUp(x_int64 bytes, x_int64 alignNum)
{
	return (bytes + (alignNum - 1) & ~(alignNum - 1));
}

x_uint64 RoundUp(x_int64 byteSize)
{
	/*if (byteSize <= 4)
	{
		return _RoundUp(byteSize, 16);
	}
	else if (byteSize <= 128)
	{
		return _RoundUp(byteSize, 16);
	}
	else if (byteSize <= 512)
	{
		return _RoundUp(byteSize, 32);
	}
	else if (byteSize <= 1024)
	{
		return _RoundUp(byteSize, 64);
	}
	else if (byteSize <= 8 * 1024)
	{
		return _RoundUp(byteSize, 128);
	}
	else if (byteSize <= 64 * 1024)
	{
		return _RoundUp(byteSize, 1024);
	}
	else if (byteSize <= 256 * 1024)
	{
		return _RoundUp(byteSize, 8 * 1024);
	}
	else
	{
		return _RoundUp(byteSize, 1 << 30);
	}*/

	return RoundUpToPowerOf2(byteSize);
}

x_uint64 RoundUpToPowerOf2(x_uint64 size)
{
	if (size <= BUDDY_MIN_SIZE)
	{
		return BUDDY_MIN_SIZE;
	}

	if (size > BUDDY_MAX_SIZE)
	{
		return size;
	}

	// 向上取整到2的幂
	size--;
	size |= size >> 1;
	size |= size >> 2;
	size |= size >> 4;
	size |= size >> 8;
	size |= size >> 16;
	size++;
	return size;
}

int GetBuddyOrder(x_uint64 size)
{
	int order = 0;
	
	size >>= 4; // 除以16
	while (size > 1)
	{
		size >>= 1;
		order++;
	}
	
	return order;
}

void* GetBuddyAddress(void* address, x_uint64 size)
{
	return (void*)((x_uint64)address ^ size);
}
