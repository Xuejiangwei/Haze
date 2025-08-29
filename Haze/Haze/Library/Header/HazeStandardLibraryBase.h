#pragma once

#include "HazeLibraryDefine.h"
#include "HazeDefine.h"

#define HAZE_STD_LIB_FUNCTION(TYPE, NAME, ...) \
	static void NAME(HAZE_STD_CALL_PARAM); \
	static TYPE NAME##Call(__VA_ARGS__);

#define HAZE_STD_LIB_FUNCTION_NO_CALL(TYPE, NAME, ...) \
	static TYPE NAME(HAZE_STD_CALL_PARAM); \

#define HAZE_INIT_STANDARD_LIB() static void InitializeLib()

class HazeStandardLibraryBase
{
public:
	HazeStandardLibraryBase();

	~HazeStandardLibraryBase();

	static bool AddStdLib(STDString libName, HashMap<STDString, void(*)(HAZE_STD_CALL_PARAM)>* hashMap);

	static void InitializeStdLibs();

	static const HashMap<STDString, HashMap<STDString, void(*)(HAZE_STD_CALL_PARAM)>*>& GetStdLib();

	static int GetStdLibSize();

private:
	static HashMap<STDString, HashMap<STDString, void(*)(HAZE_STD_CALL_PARAM)>*>* s_Hash_MapStdLib;
};
