#include <filesystem>

#include "HazeLog.h"
#include "HazeFilePathHelper.h"
#include "HazeLibraryManager.h"

extern std::wstring RootCodePath;
extern std::unique_ptr<HazeLibraryManager> HazeLibManager;

HAZE_STRING GetModuleFilePath(const HAZE_STRING& m_ModuleName)
{
	const HAZE_STRING* Path = HazeLibManager->TryGetFilePath(m_ModuleName);
	if (Path)
	{
		return *Path;
	}

	std::filesystem::path FilePath = RootCodePath + HAZE_STANDARD_FOLDER + m_ModuleName + HAZE_FILE_SUFFIX;
	if (std::filesystem::exists(FilePath))
	{
		return FilePath.c_str();
	}

	FilePath = RootCodePath + m_ModuleName + HAZE_FILE_SUFFIX;
	if (std::filesystem::exists(FilePath))
	{
		return FilePath.c_str();
	}

	HAZE_LOG_ERR_W("未能找到<%s>模块的文件路径!\n", m_ModuleName.c_str());
	return HAZE_TEXT("None");
}

HAZE_STRING GetMainBinaryFilePath()
{
	return RootCodePath + HAZE_FILE_PATH_BIN + HAZE_FILE_MAIN_BIN;
}

HAZE_STRING GetIntermediateModuleFile(const HAZE_STRING& m_ModuleName)
{
	return RootCodePath + HAZE_FILE_INTER + m_ModuleName + HAZE_FILE_INTER_SUFFIX;
}