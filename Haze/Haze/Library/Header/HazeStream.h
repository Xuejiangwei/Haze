#pragma once

#include "HazeStandardLibraryBase.h"
#include "Haze.h"

class HazeStream : public HazeStandardLibraryBase
{
public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION(void, HazePrint, int V);

	HAZE_STD_LIB_FUNCTION(void, HazePrintf, const HAZE_CHAR* V);

	HAZE_STD_LIB_FUNCTION(void, HazeScanf);
};