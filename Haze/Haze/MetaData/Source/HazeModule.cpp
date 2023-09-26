#include <filesystem>
#include <fstream>

#include "HazeModule.h"
#include "HazeLog.h"
#include "HazeCompilerValue.h"

HazeModule::HazeModule(const HAZE_STRING& opFile)
{
	ParseOpFile(opFile);
}

HazeModule::~HazeModule()
{
}

void HazeModule::ParseOpFile(const HAZE_STRING& opFile)
{
	HAZE_IFSTREAM fs(opFile);
	//FS.imbue(std::locale("chs"));
	HAZE_STRING content(std::istreambuf_iterator<HAZE_CHAR>(fs), {});

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
		HAZE_LOG_ERR(HAZE_TEXT("���ȫ�ֱ����ظ���"));
		return nullptr;
	}

	MapGlobalVariables[HAZE_TEXT("1")] = HazeCompilerValue();
	return &MapGlobalVariables[HAZE_TEXT("1")];*/
	return nullptr;
}