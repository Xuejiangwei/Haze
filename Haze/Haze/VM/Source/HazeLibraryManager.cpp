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

extern std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> Hash_MapStdLib;

std::unique_ptr<HazeLibraryManager> HazeLibManager = std::make_unique<HazeLibraryManager>();

void ExeFunc()
{
}

HazeLibraryManager::HazeLibraryManager()
{
}

HazeLibraryManager::~HazeLibraryManager()
{
	for (auto& Iter : HashMap_Library)
	{
		HAZE_UNLOAD_DLL(Iter.second.Address);
	}
}

void HazeLibraryManager::ExecuteDLLFunction(const HAZE_STRING& m_ModuleName, const HAZE_STRING& FunctionName, char* ParamStartAddress, char* RetStartAddress, void* Stack,
	void(*ExeHazeFunctionCall)(void*, void*, int, ...))
{
	auto Iter = HashMap_Library.find(m_ModuleName);
	if (Iter != HashMap_Library.end())
	{
		ExeFuncType Func = (ExeFuncType)HAZE_GET_DLL_FUNCTION(Iter->second.Address, "ExecuteFunction");
		if (Func)
		{
			if (Func(FunctionName.c_str(), ParamStartAddress, RetStartAddress, Stack, ExeHazeFunctionCall) == EXE_FUNC_ERR)
			{
				HAZE_LOG_ERR_W("执行三方库<%s>中函数<%s>返回错误代码!\n", m_ModuleName.c_str(), FunctionName.c_str());
			}
		}
		else
		{
			HAZE_LOG_ERR_W("调用三方库<%s>中函数<%s>错误,未能找到!\n", m_ModuleName.c_str(), FunctionName.c_str());
		}
	}
}

void HazeLibraryManager::LoadDLLLibrary(const HAZE_STRING& LibraryPath, const HAZE_STRING& FilePath)
{
	auto Iter = HashMap_Library.find(LibraryPath);
	if (Iter == HashMap_Library.end())
	{
		auto m_Name = GetModuleNameByFilePath(FilePath);
		auto Address = HAZE_LOAD_DLL(LibraryPath.c_str());
		if (Address != nullptr)
		{
			HashMap_Library[m_Name] = { LibraryLoadState::Load,  Address, std::move(LibraryPath), std::move(FilePath) };
		}
		else
		{
			HAZE_LOG_ERR_W("导入三方库<%s>失败!\n", LibraryPath.c_str());
		}
	}
}

void HazeLibraryManager::UnloadDLLLibrary(const HAZE_STRING& LibraryPath)
{
	auto Iter = HashMap_Library.find(LibraryPath);
	if (Iter != HashMap_Library.end())
	{
		HAZE_UNLOAD_DLL(Iter->second.Address);
	}
}

const HAZE_STRING* HazeLibraryManager::TryGetFilePath(const HAZE_STRING& m_ModuleName)
{
	auto Iter = HashMap_Library.find(m_ModuleName);
	if (Iter != HashMap_Library.end())
	{
		return &Iter->second.FilePath;
	}

	return nullptr;
}

const void* HazeLibraryManager::GetExeAddress()
{
	static uint64 Address = (uint64)(&ExeFunc);
	return &Address;
}

void HazeLibraryManager::LoadStdLibrary()
{
	if (Hash_MapStdLib.size() == 0)
	{
		HazeStream::InitializeLib();
	}
}