#include "HazeFilePathHelper.h"

extern std::wstring RootCodePath;

HAZE_STRING GetModuleFilePath(const HAZE_STRING& ModuleName)
{
	return RootCodePath + ModuleName + HAZE_FILE_SUFFIX;
}

HAZE_STRING GetMainBinaryFilePath()
{
	return RootCodePath + HAZE_FILE_PATH_BIN + HAZE_FILE_MAIN_BIN;
}

HAZE_STRING GetIntermediateModuleFile(const HAZE_STRING& ModuleName)
{
	return RootCodePath + HAZE_FILE_INTER + ModuleName + HAZE_FILE_INTER_SUFFIX;
}