#pragma once

#include "Haze.h"

class MemoryPage;

class HazeMemory
{
public:
	friend class GarbageCollection;

	HazeMemory();
	~HazeMemory();

	static std::unique_ptr<HazeMemory>& GetMemory();

	static void* Alloca(uint64 Size);

private:
	std::unordered_map<uint64, std::unique_ptr<MemoryPage>> HashMap_Page;
};

