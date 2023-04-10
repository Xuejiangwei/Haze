#pragma once

#include "HazeStandardLibraryBase.h"
#include "Haze.h"


class HazeStream : public HazeStandardLibraryBase
{
public:
	HAZE_STD_LIB_FUNCTION(void, HazePrint, int V);

	HAZE_STD_LIB_FUNCTION(void, HazePrintFloat, float V);

	HAZE_STD_LIB_FUNCTION(void, HazePrintString, const HAZE_STRING& V);

	HAZE_STD_LIB_FUNCTION(void, HazePrintf, const HAZE_CHAR* V);
};