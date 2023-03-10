#include <stdio.h>
#include <windows.h>

#include "HazeStream.h"

static std::unordered_map<HAZE_STRING, void(*)(int)> HashMap_Function =
{
	{ HAZE_TEXT("¥Ú”°"), &HazeStream::HazePrint }
};

static bool Z_NoUse_HazeStream = HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeStream"), &HashMap_Function);

//void HazeStream::HazePrint(const HAZE_CHAR* Format, ...)
//{
//	HAZE_STRING WS;
//	WS = Format;
//	std::string S = WString2String(WS);
//
//	va_list st;
//	va_start(st, Format);
//	vfprintf(stdout, S.c_str(), st);
//	va_end(st);
//}

void HazeStream::HazePrint(int V)
{
	std::cout << V << std::endl;
}
