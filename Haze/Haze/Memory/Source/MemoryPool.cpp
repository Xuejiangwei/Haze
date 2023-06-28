#include "HazeLog.h"

#include "MemoryPool.h"
#include "MemoryBucket.h"

MemoryPool::MemoryPool()
{
	/*Bucket_4Byte = std::make_unique<MemoryBucket>(4, 128);
	Bucket_8Byte = std::make_unique<MemoryBucket>(8, 128);
	Bucket_16Byte = std::make_unique<MemoryBucket>(16, 64);
	Bucket_32Byte = std::make_unique<MemoryBucket>(32, 32);
	Bucket_64Byte = std::make_unique<MemoryBucket>(64, 16);
	Bucket_128Byte = std::make_unique<MemoryBucket>(128, 8);
	Bucket_256Byte = std::make_unique<MemoryBucket>(256, 8);
	Bucket_AnyByte = std::make_unique<MemoryBucket>(-1, 0);*/
}

MemoryPool::~MemoryPool()
{
}

void* MemoryPool::Alloca(uint32 Size)
{
	void* Ret = nullptr;
	Size = (uint32)RoundUp(Size);

	uint32 Idx = (uint32)Index(Size);

	if (!Buckets[Idx])
	{
		Buckets[Idx] = std::make_unique<MemoryBucket>(Size, 128);
	}

	Ret = Buckets[Idx]->Alloca(Size);

	if (Ret == nullptr)
	{
		uint32 NextIdx = (uint32)Index(RoundUp(Size + 1));
		while (++Idx < NextIdx && Ret == nullptr)
		{
			if (Buckets[Idx])
			{
				Ret = Buckets[Idx]->Alloca(Size);
			}
		}
	}

	return Ret;

	/*static std::unordered_map<uint32, void* (MemoryPool::*)()> HashMap_Alloca =
	{
		{4, &MemoryPool::Alloca_4_Byte},
		{8, &MemoryPool::Alloca_8_Byte},
		{16, &MemoryPool::Alloca_16_Byte},
		{32, &MemoryPool::Alloca_32_Byte},
		{64, &MemoryPool::Alloca_64_Byte},
		{128, &MemoryPool::Alloca_128_Byte},
	};

	auto Iter = HashMap_Alloca.find(Size);
	if (Iter != HashMap_Alloca.end())
	{
		return (this->*Iter->second)();
	}

	return Alloca_Any_Byte();*/
}

void* MemoryPool::Alloca_4_Byte()
{
	void* Ret = Bucket_4Byte->Alloca();
	if (!Ret)
	{
		Ret = Alloca_8_Byte();
	}

	return Ret;
}

void* MemoryPool::Alloca_8_Byte()
{
	void* Ret = Bucket_8Byte->Alloca();
	if (!Ret)
	{
		Ret = Alloca_16_Byte();
	}

	return Ret;
}

void* MemoryPool::Alloca_16_Byte()
{
	void* Ret = Bucket_16Byte->Alloca();
	if (!Ret)
	{
		Ret = Alloca_32_Byte();
	}

	return Ret;
}

void* MemoryPool::Alloca_32_Byte()
{
	void* Ret = Bucket_32Byte->Alloca();
	if (!Ret)
	{
		Ret = Alloca_64_Byte();
	}

	return Ret;
}

void* MemoryPool::Alloca_64_Byte()
{
	void* Ret = Bucket_64Byte->Alloca();
	if (!Ret)
	{
		Ret = Alloca_128_Byte();
	}

	return Ret;
}

void* MemoryPool::Alloca_128_Byte()
{
	void* Ret = Bucket_128Byte->Alloca();
	if (!Ret)
	{
		Ret = Alloca_Any_Byte();
	}

	return Ret;
}

void* MemoryPool::Alloca_256_Byte()
{
	void* Ret = Bucket_256Byte->Alloca();
	if (!Ret)
	{
		Ret = Alloca_Any_Byte();
	}

	return Ret;
}

void* MemoryPool::Alloca_Any_Byte()
{
	return Bucket_AnyByte->Alloca();
}

void MemoryPool::ReleaseAll()
{
	Bucket_4Byte = nullptr;
	Bucket_8Byte = nullptr;
	Bucket_16Byte = nullptr;
	Bucket_32Byte = nullptr;
	Bucket_64Byte = nullptr;
	Bucket_128Byte = nullptr;
	Bucket_AnyByte = nullptr;
}