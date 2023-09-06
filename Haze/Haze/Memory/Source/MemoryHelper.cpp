#include "MemoryHelper.h"

static int64 _RoundUp(int64 Bytes, int64 AlignNum)
{
	return (Bytes + (AlignNum - 1) & ~(AlignNum - 1));
}

int64 RoundUp(int64 ByteSize)
{
	if (ByteSize <= 4)
	{
		return _RoundUp(ByteSize, 16);
	}
	else if (ByteSize <= 128)
	{
		return _RoundUp(ByteSize, 16);
	}
	else if (ByteSize <= 1024)
	{
		return _RoundUp(ByteSize, 16);
	}
	else if (ByteSize <= 8 * 1024)
	{
		return _RoundUp(ByteSize, 128);
	}
	else if (ByteSize <= 64 * 1024)
	{
		return _RoundUp(ByteSize, 1024);
	}
	else if (ByteSize <= 256 * 1024)
	{
		return _RoundUp(ByteSize, 8 * 1024);
	}
	else
	{
		return _RoundUp(ByteSize, 1 << 30);
	}
}