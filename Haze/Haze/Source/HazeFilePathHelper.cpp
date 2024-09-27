#include "HazePch.h"
#include <filesystem>
#include "HazeFilePathHelper.h"
#include "HazeLibraryManager.h"

extern std::wstring g_HazeExePath;
extern std::wstring g_MainFilePath;
extern Unique<HazeLibraryManager> g_HazeLibManager;

HString GetModuleFilePathByLibPath(const HString& moduleName, const HString* refModulePath, const HString* dir)
{
	const HString* path = g_HazeLibManager->TryGetFilePath(moduleName);
	if (path)
	{
		return *path;
	}

	HString filePath;
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

		filePath = g_HazeExePath + HAZE_LIB_FOLDER + dirs[0] + HAZE_FILE_SUFFIX;
		if (std::filesystem::exists(filePath))
		{
			return filePath.c_str();
		}

		filePath = g_HazeExePath + HAZE_DLL_LIB_FOLDER + dirs[0] + HAZE_FILE_SUFFIX;
		if (std::filesystem::exists(filePath))
		{
			return filePath.c_str();
		}

		if (refModulePath)
		{
			filePath = *refModulePath + dirs[0] + HAZE_FILE_SUFFIX;
			if (std::filesystem::exists(filePath))
			{
				return filePath.c_str();
			}
		}
	}

	return HString();
}

HString GetModuleFilePathByParentPath(const HString& parentPath, const HString& moduleName, const HString* refModulePath, const HString* dir)
{
	const HString* path = g_HazeLibManager->TryGetFilePath(moduleName);
	if (path)
	{
		return *path;
	}

	std::filesystem::path filePath = parentPath + HAZE_LIB_FOLDER + moduleName + HAZE_FILE_SUFFIX;
	if (std::filesystem::exists(filePath))
	{
		return filePath.c_str();
	}

	filePath = parentPath + moduleName + HAZE_FILE_SUFFIX;
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

		filePath = parentPath + dirs[0] + HAZE_FILE_SUFFIX;
		if (std::filesystem::exists(filePath))
		{
			return filePath.c_str();
		}

		if (refModulePath)
		{
			filePath = *refModulePath + dirs[0] + HAZE_FILE_SUFFIX;
			if (std::filesystem::exists(filePath))
			{
				return filePath.c_str();
			}
		}
	}

	return HString();
}


HString GetModuleFilePath(const HString& moduleName, const HString* refModulePath, const HString* dir)
{
	auto path = GetModuleFilePathByLibPath(moduleName, refModulePath, dir);
	if (path.empty())
	{
		path = GetModuleFilePathByParentPath(g_MainFilePath, moduleName, refModulePath, dir);
	}

	if (path.empty())
	{
		HAZE_LOG_ERR_W("δ���ҵ�<%s>ģ����ļ�·��!\n", moduleName.c_str());
	}

	return path;
}

HString GetMainBinaryFilePath()
{
	return g_MainFilePath + HAZE_FILE_PATH_BIN + HAZE_FILE_MAIN_BIN;
}

HString GetIntermediateModuleFile(const HString& moduleName)
{
	return g_MainFilePath + HAZE_FILE_INTER + moduleName + HAZE_FILE_INTER_SUFFIX;
}