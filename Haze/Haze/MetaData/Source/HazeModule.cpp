#include "HazePch.h"
#include <filesystem>
#include <fstream>

#include "HazeModule.h"
#include "HazeLog.h"
#include "HazeCompilerValue.h"

HazeModule::HazeModule(const HString& opFile)
{
	ParseOpFile(opFile);
}

HazeModule::~HazeModule()
{
}

void HazeModule::ParseOpFile(const HString& opFile)
{
	HAZE_IFSTREAM fs(opFile);
	//FS.imbue(std::locale("chs"));
	HString content(std::istreambuf_iterator<HChar>(fs), {});

	/*Content

	CodeText = std::move(Content);
	CurrCode = CodeText.c_str();
	FS.close();*/
}

HazeValue* HazeModule::AddGlobalVariable()
{
	/*auto It = MapGlobalVariables.find(H_TEXT(""));
	if (It != MapGlobalVariables.end())
	{
		HAZE_LOG_ERR(H_TEXT("添加全局变量重复，"));
		return nullptr;
	}

	MapGlobalVariables[H_TEXT("1")] = HazeCompilerValue();
	return &MapGlobalVariables[H_TEXT("1")];*/
	return nullptr;
}