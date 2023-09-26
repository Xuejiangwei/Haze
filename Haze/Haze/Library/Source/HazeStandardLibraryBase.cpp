#include "HazeStandardLibraryBase.h"

std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> g_Hash_MapStdLib;

HazeStandardLibraryBase::HazeStandardLibraryBase()
{
}

HazeStandardLibraryBase::~HazeStandardLibraryBase()
{
}

bool HazeStandardLibraryBase::AddStdLib(HAZE_STRING LibName, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>* HashMap)
{
	auto Iter = g_Hash_MapStdLib.find(LibName);
	if (Iter == g_Hash_MapStdLib.end())
	{
		g_Hash_MapStdLib[LibName] = HashMap;
	}

	return true;
}