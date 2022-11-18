#include "HazeLog.h"

#include "HazeCompilerFunction.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module) : Module(Module)
{
}

HazeCompilerFunction::~HazeCompilerFunction()
{
}

HazeCompilerValue* HazeCompilerFunction::AddLocalVariable(HazeCompilerModule* Module)
{
	auto It = MapLocalVariable.find(HAZE_TEXT(""));
	if (It != MapLocalVariable.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("���������� ��Ӿֲ������ظ�"));
		return nullptr;
	}

	MapLocalVariable[HAZE_TEXT("1")] = HazeCompilerValue(Module);

	return &MapLocalVariable[HAZE_TEXT("1")];
}