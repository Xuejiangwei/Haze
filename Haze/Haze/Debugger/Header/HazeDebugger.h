#pragma once

#include "HazeDebugInfo.h"
#include <unordered_set>

#include "OpenJson/openjson.h"

class HazeVM;

class HazeDebugger
{
public:
	friend class HazeStack;

	HazeDebugger(HazeVM* VM, void(* EndCall)());
	~HazeDebugger();

	enum DebuggerHookType
	{
		Line = 1,
		Instruction = 1 << 1,
		Function = 1 << 2,
	};

	void SetHook(void(* HookCall)(HazeVM* VM), uint32 Type);

	void AddBreakPoint(const char* Message);

	void DeleteBreakPoint(const char* Message);

	void DeleteAllBreakPoint(const char* Message);

	void OnExecLine(uint32 Line);

	void Start();

	void StepOver();

	void StepIn();

	void StepInstruction();

	void Continue();

	void GetLocalVariable(open::OpenJson& json);

private:
	HazeVM* VM;
	void(*HookFunctionCall)(HazeVM* VM);
	void(*EndCall)();
	uint32 HookType;

	std::unordered_map<HAZE_STRING, std::unordered_set<uint32>> HashMap_BreakPoint;

	std::pair<HAZE_STRING, uint32> CurrPauseModule;
	bool IsPause;
};

