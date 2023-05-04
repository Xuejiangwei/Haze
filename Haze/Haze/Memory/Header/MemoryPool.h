#pragma once

#include <memory>

#include "Haze.h"

#define BUCKET_NUM 208

class MemoryBucket;

class MemoryPool
{
public:
	friend class HazeStack;

	MemoryPool();
	~MemoryPool();

	//MemoryPool& operator=(const MemoryPool& Pool) = delete;
	
	//MemoryPool& operator=(const MemoryPool&& Pool) = delete;

	static int64 _RoundUp(int64 Bytes, int64 AlignNum)
	{
		return (Bytes + (AlignNum - 1) & ~(AlignNum - 1));
	}

	static int64 RoundUp(int64 ByteSize)
	{
		if (ByteSize <= 4)
		{
			return _RoundUp(ByteSize, 4);
		}
		else if (ByteSize <= 128)
		{
			return _RoundUp(ByteSize, 8);
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
		else if(ByteSize <= 256 * 1024)
		{
			return _RoundUp(ByteSize, 8 * 1024);
		}
		else
		{
			return _RoundUp(ByteSize, 1 << 30);
		}
	}

	static uint64 _Index(uint64 Bytes, uint64 AlignShift)
	{
		return ((Bytes + ((uint64)1 << AlignShift) - 1) >> AlignShift) - 1;
	}

	static uint64 Index(uint64 Size)
	{
		static int group_array[4] = { 16,56,56,56 };

		if (Size <= 128)
		{
			return _Index(Size, 3);
		}
		else if (Size <= 1024)
		{
			return _Index(Size - 128, 4) + group_array[0];
		}
		else if (Size <= 8 * 1024)
		{
			return _Index(Size - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (Size <= 64 * 1024)
		{
			return _Index(Size - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}
		else if (Size <= 256 * 1024)
		{
			return _Index(Size - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}
		else
		{
			//assert(false);
			//HAZE_LOG_ERR(HAZE_TEXT("not find memory bucket index !\n"));
			return 0;
		}
	}

private:
	void* Alloca(uint32 Size = 0);

	void* Alloca_4_Byte();
	void* Alloca_8_Byte();
	void* Alloca_16_Byte();
	void* Alloca_32_Byte();
	void* Alloca_64_Byte();
	void* Alloca_128_Byte();
	void* Alloca_256_Byte();
	void* Alloca_Any_Byte();

	void ReleaseAll();

private:
	std::unique_ptr<MemoryBucket> Buckets[BUCKET_NUM];
	std::unique_ptr<MemoryBucket> Bucket_4Byte;
	std::unique_ptr<MemoryBucket> Bucket_8Byte;
	std::unique_ptr<MemoryBucket> Bucket_16Byte;
	std::unique_ptr<MemoryBucket> Bucket_32Byte;
	std::unique_ptr<MemoryBucket> Bucket_64Byte;
	std::unique_ptr<MemoryBucket> Bucket_128Byte;
	std::unique_ptr<MemoryBucket> Bucket_256Byte;
	std::unique_ptr<MemoryBucket> Bucket_AnyByte;
};

