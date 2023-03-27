#include <stdio.h>
#include <windows.h>

#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeStream.h"

static std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> HashMap_Function =
{
	{ HAZE_TEXT("´òÓ¡"), &HazeStream::HazePrint },
	{ HAZE_TEXT("´òÓ¡×Ö·û"), &HazeStream::HazePrintString },
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
		Size = GetSizeByType(Data->Vector_Param[i].Type.Type);
		memcpy(&V, (void*)Stack->GetAddressByEBP(-Size - HAZE_ADDRESS_SIZE), Size);
	}

	HazePrintCall(V);
}

void HazeStream::HazePrintCall(int V)
{
	std::cout << V << std::endl;
}

void HazeStream::HazePrintFloat(HAZE_STD_CALL_PARAM)
{
	float V;
	int Size = 0;
	for (int i = (int)Data->Vector_Param.size() - 1; i >= 0; --i)
	{
		Size = GetSizeByType(Data->Vector_Param[i].Type.Type);
		memcpy(&V, (void*)Stack->GetAddressByEBP(-Size - HAZE_ADDRESS_SIZE), Size);
	}

	HazePrintFloatCall(V);
}

void HazeStream::HazePrintFloatCall(float V)
{
	std::wcout << V << std::endl;
}

void HazeStream::HazePrintString(HAZE_STD_CALL_PARAM)
{
	int V;
	int Size = 0;
	for (int i = (int)Data->Vector_Param.size() - 1; i >= 0; --i)
	{
		Size = GetSizeByType(Data->Vector_Param[i].Type.Type);
		memcpy(&V, (void*)Stack->GetAddressByEBP(-Size - HAZE_ADDRESS_SIZE), Size);
	}

	HazePrintStringCall(Stack->GetVM()->GetHazeStringByIndex(V));
}

void HazeStream::HazePrintStringCall(const HAZE_STRING& V)
{
	std::wcout << V << std::endl;
}

