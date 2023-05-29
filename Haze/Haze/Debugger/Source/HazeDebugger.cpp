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
			HAZE_LOG_ERR(HAZE_TEXT("ɾ���ϵ����,��ģ��<%s>δ���ҵ�<%d>��!\n"), FileName.c_str(), Line);
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("ɾ���ϵ����,δ���ҵ�ģ��<%s>!\n"), FileName.c_str());
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
