#include <filesystem>

#include "HazeLog.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"

HazeCompilerModule::HazeCompilerModule(const HAZE_STRING& ModuleName)
{
	std::wstring Path = std::filesystem::current_path();
	Path += HAZE_TEXT("\\HazeOpCodeFile\\");
	Path += HAZE_TEXT(".Hzb");

	FS.imbue(std::locale("chs"));
	FS.open(Path);
}

HazeCompilerModule::~HazeCompilerModule()
{
	FS.close();
}

HazeCompilerValue* HazeCompilerModule::AddGlobalVariable()
{
	auto It = MapGlobalVariable.find(HAZE_TEXT(""));
	if (It != MapGlobalVariable.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("编译器错误 添加全局变量重复"));
		return nullptr;
	}

	MapGlobalVariable[HAZE_TEXT("1")] = HazeCompilerValue(this);

	return &MapGlobalVariable[HAZE_TEXT("1")];
}

HazeCompilerValue* HazeCompilerModule::AddLocalVariable()
{
	return CurrFunction->AddLocalVariable(this);
}