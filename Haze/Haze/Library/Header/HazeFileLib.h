#pragma once

#include "HazeStandardLibraryBase.h"
#include "HazeHeader.h"

class HazeFileLib : public HazeStandardLibraryBase
{
public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION_NO_CALL(int*, OpenFile, const HAZE_CHAR* path, int operatorType);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, CloseFile, int* file);

	HAZE_STD_LIB_FUNCTION_NO_CALL(int, ReadChar, int* file);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, ReadString, int* file, int maxNum, const HAZE_CHAR* str);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, Read, int* file, uint64 size, uint64 count, const HAZE_CHAR* buffer);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, WriteChar, int* file, int character);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, WriteString, int* file, const HAZE_CHAR* str);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, Write, int* file, uint64 size, uint64 count, const HAZE_CHAR* buffer);
};