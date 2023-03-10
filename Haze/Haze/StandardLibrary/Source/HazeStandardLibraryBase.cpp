#include "HazeStandardLibraryBase.h"

std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(int)>*> Hash_MapStdLib;

HazeStandardLibraryBase::HazeStandardLibraryBase()
{
}

HazeStandardLibraryBase::~HazeStandardLibraryBase()
{
}

bool HazeStandardLibraryBase::AddStdLib(HAZE_STRING LibName, std::unordered_map<HAZE_STRING, void(*)(int)>* HashMap)
{
	auto Iter = Hash_MapStdLib.find(LibName);
	if (Iter == Hash_MapStdLib.end())
	{
		Hash_MapStdLib[LibName] = HashMap;
	}

	return true;
}
