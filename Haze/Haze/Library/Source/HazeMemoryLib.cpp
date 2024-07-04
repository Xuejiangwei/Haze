#include <stdio.h>
#include <windows.h>

#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeMemoryLib.h"

static std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ HAZE_TEXT("内存复制"), &HazeMemoryLib::MemoryCopy },
	{ HAZE_OBJECT_ARRAY_CONSTRUCTOR, &HazeMemoryLib::ObjectArrayConstructor },
	{ HAZE_TEXT("获得字符个数"), &HazeMemoryLib::StringCount }
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
	memcpy(dst, src, size);
}

void HazeMemoryLib::ObjectArrayConstructor(HAZE_STD_CALL_PARAM)
{
	char* address;
	char* constructorAddress;
	uint64 objectSize;
	uint64 count;

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

void HazeMemoryLib::StringCount(HAZE_STD_CALL_PARAM)
{
	HAZE_CHAR* src;

	GET_PARAM_START();
	GET_PARAM(src);

	HAZE_STRING s(src);
	int64 length = s.length();

	SET_RET_BY_TYPE(HazeValueType::Long, length);
}