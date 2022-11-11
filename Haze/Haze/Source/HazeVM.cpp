#include "HazeVM.h"

#include <iostream>
#include <locale>

HazeVM::HazeVM()
{
	std::wcout.imbue(std::locale("chs"));
}

HazeVM::~HazeVM()
{
}

void HazeVM::DoString(const std::wstring& String)
{
	Parse(String);
}

void HazeVM::Parse(const std::wstring& String)
{
	const wchar_t* Char = String.c_str();
	while (*Char != EOF)
	{
		std::wcout << *Char << std::endl;
		Char++;
	}
}