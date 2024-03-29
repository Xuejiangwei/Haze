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
};

static bool Z_NoUse_HazeMemoryLib = HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeMemoryLib"), &s_HashMap_Functions);

void HazeMemoryLib::InitializeLib()
{
	HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeMemoryLib"), &s_HashMap_Functions);
}

void HazeMemoryLib::MemoryCopy(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	void* dst;
	void* src;
	int64 size = 0;

	GET_PARAM_START();
	GET_PARAM(dst, address);
	GET_PARAM(src, address);
	GET_PARAM(size, address);
	MemoryCopyCall(dst, src, size);
}

void HazeMemoryLib::MemoryCopyCall(void* dst, const void* src, int64 size)
{
	memcpy(dst, src, size);
}