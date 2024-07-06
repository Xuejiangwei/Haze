#pragma once

#include "HazeStandardLibraryBase.h"

class HazeStream : public HazeStandardLibraryBase
{
public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION(void, HazePrintf, const HChar* v);

	HAZE_STD_LIB_FUNCTION(void, HazeScanf);
};