#include "HazeDebugger.h"
#include "HazeLog.h"
#include "HazeVM.h"
#include "HazeStack.h"

static HAZE_STRING GetFileName(const HAZE_CHAR*& Msg)
{
	bool IsNewLine = false;
	HAZE_STRING FileName;
	FileName.clear();
	while (!HazeIsSpace(*Msg, &IsNewLine))
	{
		FileName += *Msg++;
	}
	
	Msg++;

	return FileName;
}

static HAZE_STRING GetModuleName(const HAZE_STRING& FilePath)
{
	auto Index = FilePath.find_last_of(HAZE_TEXT("\\"));
	if (Index != HAZE_STRING::npos)
	{
		return FilePath.substr(Index + 1, FilePath.length() - Index - 1 - 3);
	}

	Index = FilePath.find_last_of(HAZE_TEXT("/"));
	if (Index != HAZE_STRING::npos)
	{
		return FilePath.substr(Index + 1, FilePath.length() - Index - 1 - 3);
	}

	return HAZE_TEXT("None");
}

static  HAZE_STRING GetFileModuleName(const HAZE_CHAR*& FilePath)
{
	return GetModuleName(GetFileName(FilePath));
}

void HookCall(HazeVM* VM)
{
	
}

HazeDebugger::HazeDebugger(HazeVM* VM, void(*EndCall)()) : VM(VM), HookFunctionCall(&HookCall), HookType(DebuggerHookType::Line), IsPause(true),
	EndCall(EndCall)
{
	HashMap_BreakPoint.clear();
}

HazeDebugger::~HazeDebugger()
{
	if (EndCall)
	{
		EndCall();
	}
}

void HazeDebugger::SetHook(void(*HookCall)(HazeVM* VM), uint32 Type)
{
	HookFunctionCall = HookCall;
	HookType = Type;
}

void HazeDebugger::AddBreakPoint(const char* Message)
{
	auto Msg = String2WString(Message);
	auto HazeChar = Msg.c_str();

	auto ModuleName = GetFileModuleName(HazeChar);
	uint32 Line = StringToStandardType<uint32>(HAZE_STRING(HazeChar));

	auto Iter = HashMap_BreakPoint.find(ModuleName);
	if (Iter != HashMap_BreakPoint.end())
	{
		Iter->second.insert(Line);
	}
	else
	{
		HashMap_BreakPoint[ModuleName] = { Line };
	}

	HAZE_LOG_INFO(HAZE_TEXT("添加断点<%s><%d>\n"), ModuleName.c_str(), Line);
}

void HazeDebugger::DeleteBreakPoint(const char* Message)
{
	const HAZE_CHAR* Msg = String2WString(Message).c_str();

	auto FileName = GetFileName(Msg);
	uint32 Line = StringToStandardType<uint32>(Msg);
	
	auto ModuleName = GetModuleName(FileName);
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

void HazeDebugger::DeleteAllBreakPoint(const char* Message)
{
	auto Msgs = String2WString(Message);
	auto ModuleName = GetModuleName(Msgs);

	auto Iter = HashMap_BreakPoint.find(ModuleName);
	if (Iter != HashMap_BreakPoint.end())
	{
		Iter->second.clear();
	}

	HAZE_LOG_ERR(HAZE_TEXT("清除模块<%s>的所有断点!\n"), ModuleName.c_str());
}

void HazeDebugger::OnExecLine(uint32 Line)
{
	if (!CurrPauseModule.first.empty())
	{
		if (Line == CurrPauseModule.second)
		{
			IsPause = true;
		}
		else if (Line > CurrPauseModule.second)
		{
			HAZE_LOG_ERR(HAZE_TEXT("断点错误!\n"));
		}
	}
	else
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
						if (HookFunctionCall)
						{
							HookFunctionCall(VM);
						}

						CurrPauseModule = { Iter.first, Line };
						IsPause = true;
						break;
					}
				}
			}
		}
	}

	if (IsPause)
	{
		HAZE_LOG_INFO(HAZE_TEXT("调试器暂停<%s><%d>!\n"), CurrPauseModule.first.c_str(), Line);
	}
}

void HazeDebugger::Start()
{
	IsPause = false;
}

void HazeDebugger::StepOver()
{
	if (IsPause)
	{
		IsPause = false;
		CurrPauseModule.second++;
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("单步调试错误,不是暂停状态!\n"));
	}
}

void HazeDebugger::StepIn()
{
}

void HazeDebugger::StepInstruction()
{
}

void HazeDebugger::Continue()
{
	CurrPauseModule.first.clear();
	CurrPauseModule.second = 0;
}

void HazeDebugger::GetLocalVariable(open::OpenJson& json)
{
	auto& Info = json["Info"];

	Info["Type"] = 9;

	auto& Frame = VM->VMStack->GetCurrFrame();
	if (Frame.FunctionInfo)
	{
		for (size_t i = 0; i < Frame.FunctionInfo->Vector_Variable.size(); i++)
		{
			auto Name = WString2String(Frame.FunctionInfo->Vector_Variable[i].Variable.Name);
			Info["Var"][i]["Name"] = GB2312_2_UFT8(Name.c_str());

			if (Frame.FunctionInfo->Vector_Variable[i].Variable.Type.NeedCustomName())
			{

			}
			else if (Frame.FunctionInfo->Vector_Variable[i].Variable.Type.NeedSecondaryType())
			{

			}
			else
			{
				auto Address = VM->VMStack->GetAddressByEBP(Frame.FunctionInfo->Vector_Variable[i].Offset);
				Info["Var"][i]["Value"] = GetHazeValueByBaseType<int>(Address, Frame.FunctionInfo->Vector_Variable[i].Variable.Type.PrimaryType);
			}

		}
	}
}