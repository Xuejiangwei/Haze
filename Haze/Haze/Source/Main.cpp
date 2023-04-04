// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <filesystem>

#include "Haze.h"
#include "HazeLog.h"
#include "HazeVM.h"

void HazeNewHandler()
{
	HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("Haze no memory!!!!\n"));
}

void HazeInit()
{
	std::set_new_handler(HazeNewHandler);
	setlocale(LC_ALL, "chs");
	std::wcout.imbue(std::locale("chs"));
};

//解析文本  --->  生成字节码   --->  用虚拟机解析字节码，并执行
int main()
{
	HazeInit();

	std::wstring Path = std::filesystem::current_path();

	std::filesystem::create_directory(Path + HAZE_TEXT("\\HazeOpCode"));

	HazeVM VM;
	VM.InitVM({ {Path + HAZE_TEXT("\\Other\\HazeCode.hz"), HAZE_TEXT("HazeCode")} });
	VM.LoadStandardLibrary({ {Path + HAZE_TEXT("\\Other\\HazeCode.hz"), HAZE_TEXT("HazeCode")} });
	//VM.ParseFile(Path + HAZE_TEXT("\\Other\\HazeCode.hz"), HAZE_TEXT("HazeCode"));
	VM.StartMainFunction();

	//VM.ParseString(L"Haze 测试 \n");

	std::cout << "Haze End!\n";
	getchar();
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