#include "HazeStandardLibraryBase.h"

std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> HazeStandardLibraryBase::s_Hash_MapStdLib = {};

HazeStandardLibraryBase::HazeStandardLibraryBase()
{
}

HazeStandardLibraryBase::~HazeStandardLibraryBase()
{
}

bool HazeStandardLibraryBase::AddStdLib(HAZE_STRING libName, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>* hashMap)
{
	//s_testVector.push_back(0);
	auto Iter = s_Hash_MapStdLib.find(libName);
	if (Iter == s_Hash_MapStdLib.end())
	{
		s_Hash_MapStdLib[libName] = hashMap;
	}

	return true;
}

const std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*>& HazeStandardLibraryBase::GetStdLib()
{
	return s_Hash_MapStdLib;
}

int HazeStandardLibraryBase::GetStdLibSize()
{
	return (int)s_Hash_MapStdLib.size();
}
