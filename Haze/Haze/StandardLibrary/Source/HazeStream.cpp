#include <stdio.h>
#include <windows.h>

#include "HazeStack.h"
#include "HazeStream.h"

static std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> HashMap_Function =
{
	{ HAZE_TEXT("´òÓ¡"), &HazeStream::HazePrint }
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

void HazeStream::HazePrint(HAZE_STD_CALL_PARAM)
{
	int V;
	int Size = 0;
	for (int i = (int)Data->Vector_Param.size() - 1; i >= 0; --i)
	{
		Size = GetSize(Data->Vector_Param[i].second.Type);
		memcpy(&V, (void*)Stack->GetAddressByEBP(-Size - HAZE_PUSH_ADDRESS_SIZE), Size);
	}

	HazePrintCall(V);
}

void HazeStream::HazePrintCall(int V)
{
	std::cout << V << std::endl;
}

