#pragma once
#include "HazeStandardLibraryBase.h"

class HazeMemoryLib : public HazeStandardLibraryBase
{
public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, MemoryCopy, void* dst, const void* src, int64 size);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, ObjectArrayConstructor, void* address, void* constructorAddress, uint64 objectSize, uint64 count);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, CreateClassByName, class ObjectString*);
};