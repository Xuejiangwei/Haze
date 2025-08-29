#pragma once

#include "HazeStandardLibraryBase.h"

class HazeStream : public HazeStandardLibraryBase
{
public:
	static const x_HChar* GetFormat(const x_HChar* strfrmt, x_HChar* form);

	static STDString FormatConstantString(const STDString& str);

public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION_NO_CALL(STDString, GetObjectFormatString);

	HAZE_STD_LIB_FUNCTION_NO_CALL(STDString, GetFormatString);

	HAZE_STD_LIB_FUNCTION(void, HazePrintf, const x_HChar* v);

	HAZE_STD_LIB_FUNCTION(void, HazeScanf);

	HAZE_STD_LIB_FUNCTION(void, HazeStringFormat);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, HazeLogStack);

	HAZE_STD_LIB_FUNCTION_NO_CALL(void, CreateDynamicClass);
};

class TestDynamic
{
public:
	TestDynamic() { value = 100; }
	~TestDynamic() {}

	int value;

	void Add(int a, int b) { value += (a + b); }
};

