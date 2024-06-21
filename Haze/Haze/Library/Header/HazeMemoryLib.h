#pragma once
#include "HazeStandardLibraryBase.h"
#include "HazeHeader.h"

class HazeMemoryLib : public HazeStandardLibraryBase
{
public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION(void, MemoryCopy, void* dst, const void* src, int64 size);

	HAZE_STD_LIB_FUNCTION(void, ObjectArrayConstructor, void* address, void* constructorAddress, int objectSize, int count);
};