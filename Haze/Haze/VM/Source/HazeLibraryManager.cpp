#include "HazeLibraryManager.h"

#ifdef _WIN32

	#include <Windows.h>
	#define HAZE_LOAD_DLL(X) (void*)LoadLibrary(X)
	#define HAZE_UNLOAD_DLL(X) FreeLibrary((HINSTANCE)X)
	#define HAZE_GET_DLL_FUNCTION(X, Name) GetProcAddress((HINSTANCE)X, Name)

#endif // _WIN32

using ShowHazeWindow = int(*)();

std::unique_ptr<HazeLibraryManager> HazeLibManager = std::make_unique<HazeLibraryManager>();

HazeLibraryManager::HazeLibraryManager()
{
}

HazeLibraryManager::~HazeLibraryManager()
{
	for (auto& Iter : HashMap_Library)
	{
		HAZE_UNLOAD_DLL(Iter.second.second);
	}
}

void HazeLibraryManager::GetDLLFunctionAddress(const HAZE_STRING& ModuleName, const HAZE_STRING& FunctionName)
{
	auto Iter = HashMap_Library.find(ModuleName);
	if (Iter != HashMap_Library.end())
	{
		ShowHazeWindow Func = (ShowHazeWindow)HAZE_GET_DLL_FUNCTION(Iter->second.second, "ShowOpenGLWindow"/*WString2String(FunctionName).c_str()*/);
		if (Func)
		{
			Func();
		}
	}
}

void HazeLibraryManager::LoadDLLLibrary(const HAZE_STRING& LibraryPath, const HAZE_STRING& FilePath)
{
	auto Iter = HashMap_Library.find(LibraryPath);
	if (Iter == HashMap_Library.end())
	{
		auto Name = GetModuleNameByFilePath(FilePath);
		HashMap_Library[Name] = { LibraryLoadState::Load, HAZE_LOAD_DLL(LibraryPath.c_str()) };

		/*ShowHazeWindow Func = (ShowHazeWindow)GetProcAddress((HINSTANCE)HashMap_Library[LibraryPath].second, "ShowOpenGLWindow");
		if (Func)
		{
			Func();
		}*/
	}
}

void HazeLibraryManager::UnloadDLLLibrary(const HAZE_STRING& LibraryPath)
{
	auto Iter = HashMap_Library.find(LibraryPath);
	if (Iter != HashMap_Library.end())
	{
		HAZE_UNLOAD_DLL(Iter->second.second);
	}
}