#include <iostream>
#include <locale>

#include "Haze.h"
#include "HazeVM.h"
#include "HazeCompiler.h"
#include "Parse.h"
#include "HazeLog.h"

HazeVM::HazeVM()
{
	std::wcout.imbue(std::locale("chs"));

	Compiler = std::make_unique<HazeCompiler>();
}

HazeVM::~HazeVM()
{
}

void HazeVM::ParseString(const std::wstring& String)
{
	Parse P(this);
	P.InitializeString(String);
	P.ParseContent();
}

void HazeVM::ParseFile(const std::wstring& FilePath)
{
	if (Compiler->InitializeCompiler(FilePath))
	{
		Parse P(this);
		P.InitializeFile(FilePath);
		P.ParseContent();
	}
}

VirtualRegister* HazeVM::GetVirtualRegister(int64_t Index)
{
	static std::unordered_map<int64_t, VirtualRegister*> MapRegister =
	{
		{0, &AX},
		{1, &BX},
		{2, &CX},
		{3, &DX},
	};

	auto It = MapRegister.find(Index);
	if (It == MapRegister.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("½âÎö×Ö½ÚÂëÖÐ »ñÈ¡¼Ä´æÆ÷´íÎó\n"));
	}

	return It->second;
}