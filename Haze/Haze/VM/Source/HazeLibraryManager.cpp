#include "HazeLibraryManager.h"
#include "HazeLibraryDefine.h"
#include "HazeLog.h"

#include "HazeStream.h"

#ifdef _WIN32

#include <Windows.h>
	#define HAZE_LOAD_DLL(X) (void*)LoadLibrary(X)
	#define HAZE_UNLOAD_DLL(X) FreeLibrary((HINSTANCE)X)
	#define HAZE_GET_DLL_FUNCTION(X, m_Name) GetProcAddress((HINSTANCE)X, m_Name)
#endif // _WIN32

extern std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> g_Hash_MapStdLib;

std::unique_ptr<HazeLibraryManager> g_HazeLibManager = std::make_unique<HazeLibraryManager>();

void ExeFunc()
{
}

HazeLibraryManager::HazeLibraryManager()
{
}

HazeLibraryManager::~HazeLibraryManager()
{
	for (auto& iter : m_Libraries)
	{
		HAZE_UNLOAD_DLL(iter.second.Address);
	}
}

void HazeLibraryManager::ExecuteDLLFunction(const HAZE_STRING& moduleName, const HAZE_STRING& functionName, 
	char* paramStartAddress, char* retStartAddress, void* stack, void(*ExeHazeFunctionCall)(void*, void*, int, ...))
{
	auto iter = m_Libraries.find(moduleName);
	if (iter != m_Libraries.end())
	{
		ExeFuncType func = (ExeFuncType)HAZE_GET_DLL_FUNCTION(iter->second.Address, "ExecuteFunction");
		if (func)
		{
			if (func(functionName.c_str(), paramStartAddress, retStartAddress, stack, ExeHazeFunctionCall) == EXE_FUNC_ERR)
			{
				HAZE_LOG_ERR_W("执行三方库<%s>中函数<%s>返回错误代码!\n", moduleName.c_str(), functionName.c_str());
			}
		}
		else
		{
			HAZE_LOG_ERR_W("调用三方库<%s>中函数<%s>错误,未能找到!\n", moduleName.c_str(), functionName.c_str());
		}
	}
}

void HazeLibraryManager::LoadDLLLibrary(const HAZE_STRING& libraryPath, const HAZE_STRING& filePath)
{
	auto iter = m_Libraries.find(libraryPath);
	if (iter == m_Libraries.end())
	{
		auto m_Name = GetModuleNameByFilePath(filePath);
		auto Address = HAZE_LOAD_DLL(libraryPath.c_str());
		if (Address != nullptr)
		{
			m_Libraries[m_Name] = { LibraryLoadState::Load,  Address, std::move(libraryPath), std::move(filePath) };
		}
		else
		{
			HAZE_LOG_ERR_W("导入三方库<%s>失败!\n", libraryPath.c_str());
		}
	}
}

void HazeLibraryManager::UnloadDLLLibrary(const HAZE_STRING& libraryPath)
{
	auto iter = m_Libraries.find(libraryPath);
	if (iter != m_Libraries.end())
	{
		HAZE_UNLOAD_DLL(iter->second.Address);
	}
}

const HAZE_STRING* HazeLibraryManager::TryGetFilePath(const HAZE_STRING& moduleName)
{
	auto iter = m_Libraries.find(moduleName);
	if (iter != m_Libraries.end())
	{
		return &iter->second.FilePath;
	}

	return nullptr;
}

const void* HazeLibraryManager::GetExeAddress()
{
	static uint64 address = (uint64)(&ExeFunc);
	return &address;
}

void HazeLibraryManager::LoadStdLibrary()
{
	if (g_Hash_MapStdLib.size() == 0)
	{
		HazeStream::InitializeLib();
	}
}