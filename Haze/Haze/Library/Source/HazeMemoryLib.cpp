#include <stdio.h>
#include <windows.h>

#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeMemoryLib.h"
#include "HazeLibraryDefine.h"

static std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ HAZE_TEXT("ÄÚ´æ¸´ÖÆ"), &HazeMemoryLib::MemoryCopy },
	{ HAZE_OBJECT_ARRAY_CONSTRUCTOR, &HazeMemoryLib::ObjectArrayConstructor },
};

static bool Z_NoUse_HazeMemoryLib = HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeMemoryLib"), &s_HashMap_Functions);

void HazeMemoryLib::InitializeLib()
{
	HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeMemoryLib"), &s_HashMap_Functions);
}

void HazeMemoryLib::MemoryCopy(HAZE_STD_CALL_PARAM)
{
	void* dst;
	void* src;
	int64 size = 0;

	GET_PARAM_START();
	GET_PARAM(dst);
	GET_PARAM(src);
	GET_PARAM(size);
	MemoryCopyCall(dst, src, size);
}

void HazeMemoryLib::MemoryCopyCall(void* dst, const void* src, int64 size)
{
	memcpy(dst, src, size);
}

void HazeMemoryLib::ObjectArrayConstructor(HAZE_STD_CALL_PARAM)
{
	char* address;
	char* constructorAddress;
	int objectSize;
	int count;

	GET_PARAM_START();
	GET_PARAM(address);
	GET_PARAM(constructorAddress);
	GET_PARAM(objectSize);
	GET_PARAM(count);

	for (size_t i = 0; i < count; i++)
	{
		stack->GetVM()->CallFunction((FunctionData*)constructorAddress, address + i * objectSize);
	}
}

void HazeMemoryLib::ObjectArrayConstructorCall(void* address, void* constructorAddress, int objectSize, int count)
{
}