#include "HazeLibraryManager.h"
#include "HazeLibraryDefine.h"

#include "HazeStream.h"

#ifdef _WIN32

#include <Windows.h>
#define HAZE_LOAD_DLL(X) (void*)LoadLibrary(X)
#define HAZE_UNLOAD_DLL(X) FreeLibrary((HINSTANCE)X)
#define HAZE_GET_DLL_FUNCTION(X, Name) GetProcAddress((HINSTANCE)X, Name)

#endif // _WIN32

using ExecuteFunctionType = void(*)(const wchar_t*, char*, char*);

extern std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> Hash_MapStdLib;

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

void HazeLibraryManager::ExecuteDLLFunction(const HAZE_STRING& ModuleName, const HAZE_STRING& FunctionName, char* ParamStartAddress, char* RetStartAddress)
{
	auto Iter = HashMap_Library.find(ModuleName);
	if (Iter != HashMap_Library.end())
	{
		ExecuteFunctionType Func = (ExecuteFunctionType)HAZE_GET_DLL_FUNCTION(Iter->second.second, "ExecuteFunction");
		if (Func)
		{
			Func(FunctionName.c_str(), ParamStartAddress, RetStartAddress);
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

void HazeLibraryManager::LoadStdLibrary()
{
	if (Hash_MapStdLib.size() == 0)
	{
		HazeStream::InitializeLib();
	}
}