#include <filesystem>

#include "HazeModule.h"
#include "HazeLog.h"
#include "HazeCompilerValue.h"

HazeModule::HazeModule()
{
}

HazeModule::~HazeModule()
{
}

HazeValue* HazeModule::AddGlobalVariable()
{
	/*auto It = MapGlobalVariables.find(HAZE_TEXT(""));
	if (It != MapGlobalVariables.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("添加全局变量重复，"));
		return nullptr;
	}

	MapGlobalVariables[HAZE_TEXT("1")] = HazeCompilerValue();
	return &MapGlobalVariables[HAZE_TEXT("1")];*/
	return nullptr;
}