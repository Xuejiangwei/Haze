#pragma once

#include "HazeStandardLibraryBase.h"

class HazeStream : public HazeStandardLibraryBase
{
public:
	static const HChar* GetFormat(const HChar* strfrmt, HChar* form);


public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION_NO_CALL(HString, GetFormatString);

	HAZE_STD_LIB_FUNCTION(void, HazePrintf, const HChar* v);

	HAZE_STD_LIB_FUNCTION(void, HazeScanf);
};