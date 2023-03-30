#include "HazeLog.h"

#include "MemoryPool.h"
#include "MemoryBucket.h"

MemoryPool::MemoryPool(HazeVM* VM) : VM(VM)
{
	Bucket_4Byte = std::make_unique<MemoryBucket>(4, 128);
	Bucket_8Byte = std::make_unique<MemoryBucket>(8, 128);
	Bucket_16Byte = std::make_unique<MemoryBucket>(16, 64);
	Bucket_32Byte = std::make_unique<MemoryBucket>(32, 32);
	Bucket_64Byte = std::make_unique<MemoryBucket>(64, 16);
	Bucket_128Byte = std::make_unique<MemoryBucket>(128, 8);
	Bucket_AnyByte = std::make_unique<MemoryBucket>(-1, 0);
}

MemoryPool::~MemoryPool()
{
}

void* MemoryPool::Alloca(HazeValueType Type, unsigned int Size)
{
	switch (Type)
	{
	case HazeValueType::Bool:
	case HazeValueType::Int:
	case HazeValueType::UnsignedInt:
	case HazeValueType::Float:
		return Alloca_4_Byte();
		break;
	case HazeValueType::Long:
	case HazeValueType::UnsignedLong:
	case HazeValueType::Double:
		return Alloca_8_Byte();
		break;
	case HazeValueType::Class:
	{
		if (Size < 16)
		{
			return Alloca_16_Byte();
		}
		else if (Size < 32)
		{
			return Alloca_32_Byte();
		}
		else if (Size < 64)
		{
			return Alloca_64_Byte();
		}
		else if (Size < 128)
		{
			return Alloca_128_Byte();
		}
		return Alloca_Any_Byte();
	}
		break;
	default:
		break;
	}

	return nullptr;
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

void* MemoryPool::Alloca_Any_Byte()
{
	return Bucket_AnyByte->Alloca();
}
