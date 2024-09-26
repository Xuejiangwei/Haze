#include "HazePch.h"
#include <filesystem>
#include "HazeFilePathHelper.h"
#include "HazeLibraryManager.h"

extern std::wstring g_rootCodePath;
extern Unique<HazeLibraryManager> g_HazeLibManager;

HString GetModuleFilePath(const HString& moduleName, const HString* refModulePath, const HString* dir)
{
	const HString* path = g_HazeLibManager->TryGetFilePath(moduleName);
	if (path)
	{
		return *path;
	}

	std::filesystem::path filePath = g_rootCodePath + H_TEXT("标准库\\") + moduleName + HAZE_FILE_SUFFIX;
	if (std::filesystem::exists(filePath))
	{
		return filePath.c_str();
	}

	filePath = g_rootCodePath + moduleName + HAZE_FILE_SUFFIX;
	if (std::filesystem::exists(filePath))
	{
		return filePath.c_str();
	}

	if (refModulePath)
	{
		filePath = *refModulePath + H_TEXT("\\") + moduleName + HAZE_FILE_SUFFIX;
		if (std::filesystem::exists(filePath))
		{
			return filePath.c_str();
		}
	}

	if (dir)
	{
		auto dirs = HazeStringSplit(*dir, HAZE_MODULE_PATH_CONBINE);
		for (size_t i = 0; i < dirs.size(); i++)
		{
			if (i > 0)
			{
				dirs[0] += H_TEXT("\\") + dirs[i];
			}
		}

		filePath = g_rootCodePath + dirs[0] + HAZE_FILE_SUFFIX;
		if (std::filesystem::exists(filePath))
		{
			return filePath.c_str();
		}
	}

	HAZE_LOG_ERR_W("未能找到<%s>模块的文件路径!\n", moduleName.c_str());
	return H_TEXT("None");
}

HString GetMainBinaryFilePath()
{
	return g_rootCodePath + HAZE_FILE_PATH_BIN + HAZE_FILE_MAIN_BIN;
}

HString GetIntermediateModuleFile(const HString& moduleName)
{
	return g_rootCodePath + HAZE_FILE_INTER + moduleName + HAZE_FILE_INTER_SUFFIX;
}