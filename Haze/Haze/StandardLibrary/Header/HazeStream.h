#pragma once

#include "HazeStandardLibraryBase.h"
#include "Haze.h"


class HazeStream : public HazeStandardLibraryBase
{
public:
	//static void HazePrint(const HAZE_CHAR* Format, ...);

	HAZE_STD_LIB_FUNCTION(void, HazePrint, int V);

	HAZE_STD_LIB_FUNCTION(void, HazePrintFloat, float V);

	HAZE_STD_LIB_FUNCTION(void, HazePrintString, const HAZE_STRING& V);

	HAZE_STD_LIB_FUNCTION(void, HazePrintf, const HAZE_CHAR* V);

	//static void HazePrint(HAZE_STD_CALL_PARAM);

	//static void HazePrintCall(int V);
};