#include <iostream>
#include <filesystem>

#include "Haze.h"
#include "HazeLog.h"
#include "HazeVM.h"

#include "HazeLibraryManager.h"
#include "HazeDebugger.h"

extern std::unique_ptr<HazeDebugger> g_Debugger;
extern std::unique_ptr<HazeLibraryManager> g_HazeLibManager;

bool g_IsHazeEnd = false;
std::wstring g_rootCodePath;

void HazeNewHandler()
{
	HAZE_LOG_ERR(HAZE_TEXT("Haze no memory!!!!\n"));
}

void HazePreInit()
{
	std::set_new_handler(HazeNewHandler);
	setlocale(LC_ALL, "chs");
	std::wcout.imbue(std::locale("chs"));
	std::locale::global(std::locale(std::locale::classic(), "", std::locale::ctype));
};

void HazeEnd()
{
	g_IsHazeEnd = true;
	std::cout << std::endl << std::endl << "Haze End!" << std::endl;
	system("pause");
}

void HazeExit()
{
	if (g_Debugger)
	{
		g_Debugger.release();
	}

	/*if (!IsHazeEnd)
	{
		HazeEnd();
	}*/
}

enum class ParamType
{
	MainFile,
	DebugType,
	LoadLibrary,
};

uint32 GetParam(ParamType type, char** paramArray, int length)
{
	std::unordered_map<ParamType, const char*> HashMap_Param =
	{
		{ ParamType::MainFile, "-m" },
		{ ParamType::DebugType, "-d" },
		{ ParamType::LoadLibrary, "-ld" },
	};

	auto Iter = HashMap_Param.find(type);
	if (Iter != HashMap_Param.end())
	{
		for (size_t i = 0; i < length; i++)
		{
			if (i + 1 < length && strcmp(paramArray[i], Iter->second) == 0)
			{
				return (uint32)i + 1;
			}
		}
	}

	return 0;
}

//解析文本  --->  生成字节码   --->  用虚拟机解析字节码，并执行
int HazeMain(int argCount, char* argValue[])
{
	atexit(HazeExit);

	for (size_t i = 0; i < argCount; i++)
	{
		HAZE_LOG_INFO("%s\n", argValue[i]);
	}

	HazePreInit();

	std::filesystem::path ExeFile(argValue[0]);
	//RootCodePath = ExeFile.parent_path();
	//std::wstring Path = std::filesystem::current_path();

	char* mainFilePath = nullptr;
	if (argCount < 2)
	{
		return 0;
	}
	else if (argCount == 2)
	{
		mainFilePath = argValue[1];
	}
	else
	{
		mainFilePath = argValue[GetParam(ParamType::MainFile, argValue, argCount)];
	}

	HazeRunType runType = GetParam(ParamType::DebugType, argValue, argCount) != 0 ?
		strcmp(argValue[GetParam(ParamType::DebugType, argValue, argCount)], "debug") == 0 ? HazeRunType::Debug : HazeRunType::Release : HazeRunType::Release;

	std::filesystem::path mainFile(mainFilePath);
	g_rootCodePath = mainFile.parent_path().wstring() + HAZE_TEXT("\\");

	std::filesystem::create_directory(g_rootCodePath + HAZE_FILE_INTER);
	std::filesystem::create_directory(g_rootCodePath + HAZE_FILE_PATH_BIN);

	for (int dll_Index = 0; dll_Index < argCount; dll_Index += 2)
	{
		dll_Index = GetParam(ParamType::LoadLibrary, argValue + dll_Index, argCount - dll_Index);
		if (dll_Index > 0)
		{
			g_HazeLibManager->LoadDLLLibrary(String2WString(argValue[dll_Index]), String2WString(argValue[dll_Index + 1]));
		}
		else
		{
			break;
		}
	}

	HazeVM vm(runType);

	vm.InitVM({ { mainFile , mainFile.filename().wstring().substr(0, mainFile.filename().wstring().length() - 3) } });

	//VM.LoadStandardLibrary({ {Path + HAZE_TEXT("\\Code\\HazeCode.hz"), HAZE_TEST_FILE} });
	//VM.ParseFile(Path + HAZE_TEXT("\\Other\\HazeCode.hz"), HAZE_TEXT("HazeCode"));

	std::cout << std::endl << std::endl << "Haze Start" << std::endl << std::endl;

	vm.StartMainFunction();

	HazeEnd();

	return 0;
}