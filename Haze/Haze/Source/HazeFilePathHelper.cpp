#include "HazePch.h"
#include <filesystem>
#include <chrono>
#include "HazeFilePathHelper.h"
#include "HazeLibraryManager.h"

extern std::wstring g_HazeExePath;
extern std::wstring g_MainFilePath;
extern Unique<V_Array<std::wstring>> g_CustomPaths;
extern Unique<HazeLibraryManager> g_HazeLibManager;

STDString GetModuleFilePathByLibPath(const STDString& modulePath, const STDString* refModulePath)
{
	auto modulePackage = HazeStringSplit(modulePath, HAZE_MODULE_PATH_CONBINE);
	auto path = g_HazeLibManager->TryGetFilePath(modulePackage.back());
	if (path)
	{
		return path->c_str();
	}

	STDString filePath;
	STDString modulePackagePath;
	for (auto it : modulePackage)
	{
		modulePackagePath += (H_TEXT("\\") + it);
	}
	modulePackagePath += HAZE_FILE_SUFFIX;

	if (refModulePath)
	{
		std::filesystem::path refFile(*refModulePath);
		filePath = refFile.parent_path().wstring() + modulePackagePath;
		if (std::filesystem::exists(filePath))
		{
			return filePath.c_str();
		}
	}
	
	// 可执行程序下的三方库文件路径
	filePath = g_HazeExePath + HAZE_THIRD_LIB_FOLDER + modulePackagePath;
	if (std::filesystem::exists(filePath))
	{
		return filePath.c_str();
	}

	filePath = g_HazeExePath + HAZE_THIRD_DLL_LIB_FOLDER + modulePackagePath;
	if (std::filesystem::exists(filePath))
	{
		return filePath.c_str();
	}

	if (g_CustomPaths)
	{
		for (auto& customPath : *g_CustomPaths)
		{
			filePath = customPath + modulePackagePath;
			if (std::filesystem::exists(filePath))
			{
				return filePath.c_str();
			}

			filePath = customPath + HAZE_THIRD_LIB_FOLDER + modulePackagePath;
			if (std::filesystem::exists(filePath))
			{
				return filePath.c_str();
			}

			filePath = customPath + HAZE_THIRD_DLL_LIB_FOLDER + modulePackagePath;
			if (std::filesystem::exists(filePath))
			{
				return filePath.c_str();
			}
		}
	}

	return STDString();
}

STDString GetModuleFilePathByParentPath(const STDString& parentPath, const STDString& moduleName, const STDString* refModulePath)
{
	const STDString* path = g_HazeLibManager->TryGetFilePath(moduleName);
	if (path)
	{
		return *path;
	}

	std::filesystem::path filePath = parentPath + HAZE_THIRD_LIB_FOLDER + moduleName + HAZE_FILE_SUFFIX;
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

	/*if (dir)
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
	}*/

	return STDString();
}


STDString GetModuleFilePath(const STDString& modulePath, const STDString* refModulePath)
{
	auto path = GetModuleFilePathByLibPath(modulePath, refModulePath);
	/*if (path.empty())
	{
		path = GetModuleFilePathByParentPath(g_MainFilePath, modulePackage.back().c_str(), refModulePath);
	}*/

	if (path.empty())
	{
		HAZE_LOG_ERR_W("未能找到<%s>模块的文件路径, 来自<%s>的引用!\n", modulePath.c_str(), refModulePath ? refModulePath->c_str(): H_TEXT("None"));
	}

	return path;
}

STDString GetMainBinaryFilePath()
{
	return g_MainFilePath + HAZE_FILE_PATH_BIN + HAZE_FILE_MAIN_BIN;
}

STDString GetIntermediateModuleFile(const STDString& moduleName)
{
	return g_MainFilePath + HAZE_FILE_INTER + moduleName.c_str() + HAZE_FILE_INTER_SUFFIX;
}

x_uint64 GetFileLastTime(const STDString& filePath)
{
	if (FileExist(filePath))
	{
		auto time = std::filesystem::last_write_time(filePath.c_str());
		auto sysTime = std::chrono::clock_cast<std::chrono::system_clock>(time);
		return std::chrono::duration_cast<std::chrono::milliseconds>(sysTime.time_since_epoch()).count();
	}
	
	return 0;
}

bool FileExist(const STDString& filePath)
{
	return std::filesystem::exists(filePath.c_str());
}
