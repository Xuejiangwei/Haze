#include "HazeDebugger.h"
#include "HazeLog.h"
#include "HazeVM.h"
#include "HazeStack.h"

HazeDebugger::HazeDebugger(HazeVM* VM) : VM(VM), HookFunctionCall(nullptr), HookType(0)
{
	HashMap_BreakPoint.clear();
}

HazeDebugger::~HazeDebugger()
{
}

void HazeDebugger::SetHook(void(*HookCall)(HazeVM* VM), uint32 Type)
{
	HookFunctionCall = HookCall;
	HookType = Type;
}

void HazeDebugger::AddBreakPoint(const HAZE_STRING& FileName, uint32 Line)
{
	auto Iter = HashMap_BreakPoint.find(FileName);
	if (Iter != HashMap_BreakPoint.end())
	{
		Iter->second.insert(Line);
	}
	else
	{
		HashMap_BreakPoint[FileName] = { Line };
	}
}

void HazeDebugger::DeleteBreakPoint(const HAZE_STRING& FileName, uint32 Line)
{
	auto Iter = HashMap_BreakPoint.find(FileName);
	if (Iter != HashMap_BreakPoint.end())
	{
		auto It = Iter->second.find(Line);
		if (It != Iter->second.end())
		{
			Iter->second.erase(It);
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("删除断点错误,在模块<%s>未能找到<%d>行!\n"), FileName.c_str(), Line);
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("删除断点错误,未能找到模块<%s>!\n"), FileName.c_str());
	}
}

void HazeDebugger::DeleteAllBreakPoint()
{
	HashMap_BreakPoint.clear();
}

void HazeDebugger::OnExecLine(uint32 Line)
{
	for (auto& Iter : HashMap_BreakPoint)
	{
		auto It = Iter.second.find(Line);
		if (It != Iter.second.end())
		{
			auto Name = VM->GetModuleNameByCurrFunction();
			if (*Name == Iter.first)
			{
				if (HookType & DebuggerHookType::Line)
				{
					HookFunctionCall(VM);
				}
			}

		}
	}
}

void HazeDebugger::StepOver()
{
}

void HazeDebugger::StepIn()
{
}
