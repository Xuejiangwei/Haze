#pragma once

#include "Haze.h"

#define HAZE_STD_LIB_FUNCTION(TYPE, NAME, ...) \
	static TYPE NAME(HAZE_STD_CALL_PARAM); \
	static TYPE NAME##Call(__VA_ARGS__);

class HazeStandardLibraryBase
{
public:
	HazeStandardLibraryBase();
	~HazeStandardLibraryBase();

	static bool AddStdLib(HAZE_STRING LibName, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>* HashMap);

private:

};
