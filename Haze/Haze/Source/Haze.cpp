#include <iostream>
#include <filesystem>

#include "Haze.h"
#include "HazeLog.h"
#include "HazeVM.h"

#include "HazeLibraryManager.h"
#include "HazeDebugger.h"

extern std::unique_ptr<HazeDebugger> Debugger;
extern std::unique_ptr<HazeLibraryManager> HazeLibManager;

bool IsHazeEnd = false;
std::wstring RootCodePath;

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
	IsHazeEnd = true;
	std::cout << std::endl << "Haze End!" << std::endl;
	system("pause");
}

void HazeExit()
{
	if (Debugger)
	{
		Debugger.release();
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

uint32 GetParam(ParamType Type, char** ParamArray, int Length)
{
	std::unordered_map<ParamType, const char*> HashMap_Param =
	{
		{ ParamType::MainFile, "-m" },
		{ ParamType::DebugType, "-d" },
		{ ParamType::LoadLibrary, "-ld" },
	};

	auto Iter = HashMap_Param.find(Type);
	if (Iter != HashMap_Param.end())
	{
		for (size_t i = 0; i < Length; i++)
		{
			if (i + 1 < Length && strcmp(ParamArray[i], Iter->second) == 0)
			{
				return (uint32)i + 1;
			}
		}
	}

	return 0;
}

//解析文本  --->  生成字节码   --->  用虚拟机解析字节码，并执行
int HazeMain(int ArgCount, char* ArgValue[])
{
	atexit(HazeExit);

	for (size_t i = 0; i < ArgCount; i++)
	{
		HAZE_LOG_INFO("%s\n", ArgValue[i]);
	}

	HazePreInit();

	std::filesystem::path ExeFile(ArgValue[0]);
	//RootCodePath = ExeFile.parent_path();
	//std::wstring Path = std::filesystem::current_path();

	char* MainFilePath = nullptr;
	if (ArgCount < 2)
	{
		return 0;
	}
	else if (ArgCount == 2)
	{
		MainFilePath = ArgValue[1];
	}
	else
	{
		MainFilePath = ArgValue[GetParam(ParamType::MainFile, ArgValue, ArgCount)];
	}

	HazeRunType Type = GetParam(ParamType::DebugType, ArgValue, ArgCount) != 0 ?
		strcmp(ArgValue[GetParam(ParamType::DebugType, ArgValue, ArgCount)], "debug") == 0 ? HazeRunType::Debug : HazeRunType::Release : HazeRunType::Release;

	std::filesystem::path MainFile(MainFilePath);
	RootCodePath = MainFile.parent_path().wstring() + HAZE_TEXT("\\");

	std::filesystem::create_directory(RootCodePath + HAZE_FILE_INTER);
	std::filesystem::create_directory(RootCodePath + HAZE_FILE_PATH_BIN);

	for (int DLLLibIndex = 0; DLLLibIndex < ArgCount; DLLLibIndex += 2)
	{
		DLLLibIndex = GetParam(ParamType::LoadLibrary, ArgValue + DLLLibIndex, ArgCount - DLLLibIndex);
		if (DLLLibIndex > 0)
		{
			HazeLibManager->LoadDLLLibrary(String2WString(ArgValue[DLLLibIndex]), String2WString(ArgValue[DLLLibIndex + 1]));
		}
		else
		{
			break;
		}
	}

	HazeVM VM(Type);

	VM.InitVM({ { MainFile , MainFile.filename().wstring().substr(0, MainFile.filename().wstring().length() - 3) } });

	//VM.LoadStandardLibrary({ {Path + HAZE_TEXT("\\Code\\HazeCode.hz"), HAZE_TEST_FILE} });
	//VM.ParseFile(Path + HAZE_TEXT("\\Other\\HazeCode.hz"), HAZE_TEXT("HazeCode"));

	std::cout << std::endl << std::endl << "Haze Start" << std::endl << std::endl;

	VM.StartMainFunction();

	HazeEnd();

	return 0;
}