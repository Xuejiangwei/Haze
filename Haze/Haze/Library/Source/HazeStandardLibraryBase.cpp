#include "HazeStandardLibraryBase.h"

std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> g_Hash_MapStdLib;

HazeStandardLibraryBase::HazeStandardLibraryBase()
{
}

HazeStandardLibraryBase::~HazeStandardLibraryBase()
{
}

bool HazeStandardLibraryBase::AddStdLib(HAZE_STRING libName, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>* hashMap)
{
	auto Iter = g_Hash_MapStdLib.find(libName);
	if (Iter == g_Hash_MapStdLib.end())
	{
		g_Hash_MapStdLib[libName] = hashMap;
	}

	return true;
}