#include "HazeMemory.h"
#include "MemoryPage.h"
#include "MemoryBlock.h"
#include "MemoryHelper.h"
#include "GarbageCollection.h"

#define PAGE_BASE_SIZE  4 * 4
#define MAX_HAZE_ALLOC_SIZE 2048

HazeMemory::HazeMemory()
{
}

HazeMemory::~HazeMemory()
{
}

std::unique_ptr<HazeMemory>& HazeMemory::GetMemory()
{
	static std::unique_ptr<HazeMemory> Memory = std::make_unique<HazeMemory>();
	return Memory;
}

void* HazeMemory::Alloca(uint64 Size)
{
	void* Ret = nullptr;
	if (Size <= MAX_HAZE_ALLOC_SIZE)
	{
		Size = RoundUp(Size);

		auto Iter = GetMemory()->HashMap_Page.find(Size);
		if (Iter != GetMemory()->HashMap_Page.end())
		{
			Ret = Iter->second->TryAlloca(Size);
		}
		else
		{
			uint64 PageSize = Size > PAGE_BASE_SIZE ? Size : PAGE_BASE_SIZE;
			GetMemory()->HashMap_Page[Size] = std::make_unique<MemoryPage>(PageSize, Size);

			Ret = GetMemory()->HashMap_Page[Size]->TryAlloca(Size);
		}
	}
	else
	{
		Ret = malloc(Size);
		GetMemory()->HashMap_BigMemory[Ret] = { GC_State::Black, Ret };
	}

	return Ret;
}