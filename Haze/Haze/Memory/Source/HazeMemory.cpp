#include "HazeMemory.h"
#include "MemoryPage.h"
#include "MemoryBlock.h"
#include "MemoryHelper.h"

#define PAGE_BASE_SIZE  4 * 4

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
	Size = RoundUp(Size);

	auto Iter = GetMemory()->HashMap_Page.find(Size);
	if (Iter != GetMemory()->HashMap_Page.end())
	{
		Ret = Iter->second->TryAlloca(Size);

		if (!Ret)
		{
			uint64 PageSize = Size > PAGE_BASE_SIZE ? Size : PAGE_BASE_SIZE;
			Iter->second->SetNextPage(std::make_unique<MemoryPage>(PageSize, Size));
			Ret = Iter->second->TryAlloca(Size);
		}
	}
	else
	{
		uint64 PageSize = Size > PAGE_BASE_SIZE ? Size : PAGE_BASE_SIZE;
		GetMemory()->HashMap_Page[Size] = std::make_unique<MemoryPage>(PageSize, Size);

		Ret = GetMemory()->HashMap_Page[Size]->TryAlloca(Size);
	}

	return Ret;
}
