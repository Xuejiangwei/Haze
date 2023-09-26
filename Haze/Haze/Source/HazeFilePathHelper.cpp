#include <filesystem>

#include "HazeLog.h"
#include "HazeFilePathHelper.h"
#include "HazeLibraryManager.h"

extern std::wstring g_rootCodePath;
extern std::unique_ptr<HazeLibraryManager> g_HazeLibManager;

HAZE_STRING GetModuleFilePath(const HAZE_STRING& moduleName)
{
	const HAZE_STRING* path = g_HazeLibManager->TryGetFilePath(moduleName);
	if (path)
	{
		return *path;
	}

	std::filesystem::path filePath = g_rootCodePath + HAZE_STANDARD_FOLDER + moduleName + HAZE_FILE_SUFFIX;
	if (std::filesystem::exists(filePath))
	{
		return filePath.c_str();
	}

	filePath = g_rootCodePath + moduleName + HAZE_FILE_SUFFIX;
	if (std::filesystem::exists(filePath))
	{
		return filePath.c_str();
	}

	HAZE_LOG_ERR_W("未能找到<%s>模块的文件路径!\n", moduleName.c_str());
	return HAZE_TEXT("None");
}

HAZE_STRING GetMainBinaryFilePath()
{
	return g_rootCodePath + HAZE_FILE_PATH_BIN + HAZE_FILE_MAIN_BIN;
}

HAZE_STRING GetIntermediateModuleFile(const HAZE_STRING& moduleName)
{
	return g_rootCodePath + HAZE_FILE_INTER + moduleName + HAZE_FILE_INTER_SUFFIX;
}