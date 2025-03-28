#include "HazePch.h"
#include "MemoryHelper.h"

static x_int64 _RoundUp(x_int64 bytes, x_int64 alignNum)
{
	return (bytes + (alignNum - 1) & ~(alignNum - 1));
}

x_uint64 RoundUp(x_int64 byteSize)
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