#include "HazePch.h"
#include "MemoryHelper.h"

static int64 _RoundUp(int64 bytes, int64 alignNum)
{
	return (bytes + (alignNum - 1) & ~(alignNum - 1));
}

int64 RoundUp(int64 byteSize)
{
	if (byteSize <= 4)
	{
		return _RoundUp(byteSize, 16);
	}
	else if (byteSize <= 128)
	{
		return _RoundUp(byteSize, 16);
	}
	else if (byteSize <= 1024)
	{
		return _RoundUp(byteSize, 16);
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
	}
}