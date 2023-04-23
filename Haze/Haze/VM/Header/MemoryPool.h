#pragma once

#include <memory>

#include "Haze.h"

class MemoryBucket;

class MemoryPool
{
public:
	friend class HazeStack;

	MemoryPool();
	~MemoryPool();

private:
	void* Alloca(uint32 Size = 0);

	void* Alloca_4_Byte();
	void* Alloca_8_Byte();
	void* Alloca_16_Byte();
	void* Alloca_32_Byte();
	void* Alloca_64_Byte();
	void* Alloca_128_Byte();
	void* Alloca_Any_Byte();

	void ReleaseAll();

private:
	std::unique_ptr<MemoryBucket> Bucket_4Byte;
	std::unique_ptr<MemoryBucket> Bucket_8Byte;
	std::unique_ptr<MemoryBucket> Bucket_16Byte;
	std::unique_ptr<MemoryBucket> Bucket_32Byte;
	std::unique_ptr<MemoryBucket> Bucket_64Byte;
	std::unique_ptr<MemoryBucket> Bucket_128Byte;
	std::unique_ptr<MemoryBucket> Bucket_AnyByte;
};

