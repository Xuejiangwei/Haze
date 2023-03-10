#pragma once

#include "Haze.h"

#define HAZE_STD_LIB_FUNCTION(FUNC) \
	public: \
	static void Haze_STD_LIB_##FUNC() { FUNC() }

class HazeStandardLibraryBase
{
public:
	HazeStandardLibraryBase();
	~HazeStandardLibraryBase();

	static bool AddStdLib(HAZE_STRING LibName, std::unordered_map<HAZE_STRING, void(*)(int)>* HashMap);

private:

};
