#include "HazePch.h"
#include "HazeStandardLibraryBase.h"

HashMap<STDString, HashMap<STDString, void(*)(HAZE_STD_CALL_PARAM)>*>* HazeStandardLibraryBase::s_Hash_MapStdLib = nullptr;

HazeStandardLibraryBase::HazeStandardLibraryBase()
{
}

HazeStandardLibraryBase::~HazeStandardLibraryBase()
{
}

bool HazeStandardLibraryBase::AddStdLib(STDString libName, HashMap<STDString, void(*)(HAZE_STD_CALL_PARAM)>* hashMap)
{
	if (s_Hash_MapStdLib)
	{
		//s_testVector.push_back(0);
		auto Iter = s_Hash_MapStdLib->find(libName);
		if (Iter == s_Hash_MapStdLib->end())
		{
			(*s_Hash_MapStdLib)[libName] = hashMap;
		}
	}
	else
	{
		s_Hash_MapStdLib = new HashMap<STDString, HashMap<STDString, void(*)(HAZE_STD_CALL_PARAM)>*>();
		(*s_Hash_MapStdLib)[libName] = hashMap;
	}

	return true;
}

const HashMap<STDString, HashMap<STDString, void(*)(HAZE_STD_CALL_PARAM)>*>& HazeStandardLibraryBase::GetStdLib()
{
	return *s_Hash_MapStdLib;
}

int HazeStandardLibraryBase::GetStdLibSize()
{
	return (int)s_Hash_MapStdLib->size();
}
