#pragma once

#include "HazeStandardLibraryBase.h"

class HazeFileLib : public HazeStandardLibraryBase
{
public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, OpenFile, const HChar* path, int operatorType);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, CloseFile, int* file);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, ReadChar, int* file);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, ReadString, int* file, int maxNum, const HChar* str);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, Read, int* file, uint64 size, uint64 count, const HChar* buffer);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, ReadLine, int* file, const HChar* buffer);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, WriteChar, int* file, int character);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, WriteString, int* file, const HChar* str);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, Write, int* file, uint64 size, uint64 count, const HChar* buffer);
};