#pragma once

#include "HazeDebugInfo.h"
#include <unordered_set>

class HazeVM;

class HazeDebugger
{
public:
	HazeDebugger(HazeVM* VM);
	~HazeDebugger();


	enum DebuggerHookType
	{
		Line = 1,
		Instruction = 1 << 1,
		Function = 1 << 2,
	};

	void SetHook(void(* HookCall)(HazeVM* VM), uint32 Type);

	void AddBreakPoint(const HAZE_STRING& FileName, uint32 Line);

	void DeleteBreakPoint(const HAZE_STRING& FileName, uint32 Line);

	void DeleteAllBreakPoint();

	void OnExecLine(uint32 Line);

	void StepOver();

	void StepIn();

private:
	HazeVM* VM;
	void(* HookFunctionCall)(HazeVM* VM);
	uint32 HookType;

	std::unordered_map<HAZE_STRING, std::unordered_set<uint32>> HashMap_BreakPoint;
};

