// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <filesystem>

#include "Haze.h"
#include "HazeLog.h"
#include "HazeVM.h"

#ifdef _DEBUG
	#include "HazeDebug.h"
#endif // _DEBUG

std::wstring RootCodePath;

void HazeNewHandler()
{
	HAZE_LOG_ERR(HAZE_TEXT("Haze no memory!!!!\n"));
}

void HazeInit()
{
	std::set_new_handler(HazeNewHandler);
	setlocale(LC_ALL, "chs");
	std::wcout.imbue(std::locale("chs"));
	std::locale::global(std::locale(std::locale::classic(), "", std::locale::ctype));
};

//解析文本  --->  生成字节码   --->  用虚拟机解析字节码，并执行
int main(int ArgCount, char* ArgValue[])
{
	HazeInit();

	std::filesystem::path ExeFile(ArgValue[0]);
	RootCodePath = ExeFile.parent_path();
	//std::wstring Path = std::filesystem::current_path();

	if (ArgCount < 2)
	{
		return 0;
	}

	std::filesystem::path MainFile(ArgValue[1]);
	RootCodePath = MainFile.parent_path().wstring() + HAZE_TEXT("\\");

	std::filesystem::create_directory(RootCodePath + HAZE_FILE_INTER);
	std::filesystem::create_directory(RootCodePath + HAZE_FILE_PATH_BIN);

	HazeVM VM;


#ifdef _DEBUG
	
	VM.InitVM({ {/*RootCodePath + HAZE_TEST_FOLDER + HAZE_TEST_FILE_PATH*/ MainFile , MainFile.filename().wstring().substr(0, MainFile.filename().wstring().length() - 3) /*HAZE_TEST_FILE*/ } });

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
	
	std::cout << std::endl << "Haze End!" << std::endl;
	
	getchar();
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