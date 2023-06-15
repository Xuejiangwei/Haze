// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

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
	LoadLibrary,
};

uint32 GetParam(ParamType Type, char** ParamArray, int Length)
{
	std::unordered_map<ParamType, const char*> HashMap_Param =
	{
		{ ParamType::MainFile, "-m" },
		{ ParamType::LoadLibrary, "-ld" },
	};

	auto Iter = HashMap_Param.find(Type);
	if (Iter != HashMap_Param.end())
	{

		for (size_t i = 0; i < Length; i++)
		{
			if (strcmp(ParamArray[i], Iter->second) == 0 && i + 1 < Length)
			{
				return i + 1;
			}
		}
	}

	return 0;
}

//解析文本  --->  生成字节码   --->  用虚拟机解析字节码，并执行
int main(int ArgCount, char* ArgValue[])
{
	atexit(HazeExit);

	for (size_t i = 0; i < ArgCount; i++)
	{
		HAZE_LOG_INFO("%s\n", ArgValue[i]);
	}

	HazePreInit();

	std::filesystem::path ExeFile(ArgValue[0]);
	RootCodePath = ExeFile.parent_path();
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

	std::filesystem::path MainFile(MainFilePath);
	RootCodePath = MainFile.parent_path().wstring() + HAZE_TEXT("\\");

	std::filesystem::create_directory(RootCodePath + HAZE_FILE_INTER);
	std::filesystem::create_directory(RootCodePath + HAZE_FILE_PATH_BIN);

	for (uint32 DLLLibIndex = 0; DLLLibIndex < ArgCount; DLLLibIndex += 2)
	{
		DLLLibIndex = GetParam(ParamType::LoadLibrary, ArgValue + DLLLibIndex, ArgCount);
		if (DLLLibIndex > 0)
		{
			HazeLibManager->LoadDLLLibrary(String2WString(ArgValue[DLLLibIndex]), String2WString(ArgValue[DLLLibIndex + 1]));
		}
	}

	HazeVM VM(HazeGenType::Release);


#ifdef _DEBUG
	
	VM.InitVM({ { MainFile , MainFile.filename().wstring().substr(0, MainFile.filename().wstring().length() - 3) } });

#else

	if (ArgCount == 1)
	{
		HAZE_STRING FilePath = String2WString(ArgValue[1]);
		HAZE_STRING ModuleName = FilePath.substr(0, FilePath.length() - 3);

		VM.InitVM({ {Path + HAZE_TEXT("\\") + FilePath,  ModuleName} });
	}

#endif
	//VM.LoadStandardLibrary({ {Path + HAZE_TEXT("\\Code\\HazeCode.hz"), HAZE_TEST_FILE} });
	//VM.ParseFile(Path + HAZE_TEXT("\\Other\\HazeCode.hz"), HAZE_TEXT("HazeCode"));

	std::cout << std::endl << std::endl << "Haze Start" << std::endl << std::endl;
	
	VM.StartMainFunction();
	
	HazeEnd();

	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件