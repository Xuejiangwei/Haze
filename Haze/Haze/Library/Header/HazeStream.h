#pragma once

#include "HazeStandardLibraryBase.h"
#include "HazeHeader.h"

class HazeStream : public HazeStandardLibraryBase
{
public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION(void, HazePrintf, const HAZE_CHAR* v);

	HAZE_STD_LIB_FUNCTION(void, HazeScanf);
};