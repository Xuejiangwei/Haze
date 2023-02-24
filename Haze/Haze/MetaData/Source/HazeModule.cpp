#include <filesystem>
#include <fstream>

#include "HazeModule.h"
#include "HazeLog.h"
#include "HazeCompilerValue.h"

HazeModule::HazeModule(const HAZE_STRING& OpFile)
{
	ParseOpFile(OpFile);
}

HazeModule::~HazeModule()
{
}

void HazeModule::ParseOpFile(const HAZE_STRING& OpFile)
{
	std::wifstream FS(OpFile);
	//FS.imbue(std::locale("chs"));
	HAZE_STRING Content(std::istreambuf_iterator<HAZE_CHAR>(FS), {});
	
	/*Content

	
	CodeText = std::move(Content);
	CurrCode = CodeText.c_str();
	FS.close();*/
}

HazeValue* HazeModule::AddGlobalVariable()
{
	/*auto It = MapGlobalVariables.find(HAZE_TEXT(""));
	if (It != MapGlobalVariables.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("���ȫ�ֱ����ظ���"));
		return nullptr;
	}

	MapGlobalVariables[HAZE_TEXT("1")] = HazeCompilerValue();
	return &MapGlobalVariables[HAZE_TEXT("1")];*/
	return nullptr;
}