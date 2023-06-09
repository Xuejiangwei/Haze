#pragma once

#include "HazeDebugInfo.h"
#include <unordered_set>

#include "OpenJson/openjson.h"

class HazeVM;

class HazeDebugger
{
public:
	friend class HazeVM;
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

	void DeleteModuleAllBreakPoint(const char* Message);

	void DeleteAllBreakPoint();

	void OnExecLine(uint32 Line);

	void Start();

	void StepOver();

	void StepIn();

	void StepInstruction();

	void Continue();

	void SetJsonLocalVariable(open::OpenJson& Json);
public:
	void AddTempBreakPoint(uint32 Line);

	void SendProgramEnd();

private:
	void ClearCurrParseModuleData() 
	{
		CurrPauseModule.first.clear();
		CurrPauseModule.second = 0; 
	}

	bool CurrModuleIsStepOver();

	void SetJsonType(open::OpenJson& Json, HazeDebugInfoType Type) { Json["Type"] = (int)Type; }
	
	void SetJsonBreakFilePath(open::OpenJson& Json, HAZE_STRING Path)
	{
		ReplacePathSlash(Path);
		auto Name = WString2String(Path);
		Json["BreakPathFile"] = GB2312_2_UFT8(Name.c_str());
	}
	
	void SetJsonBreakLine(open::OpenJson& Json, uint32 Line) { Json["BreakLine"] = Line; }
	
	void SendBreakInfo();

	void SetJsonVariableData(open::OpenJson& Json, const HazeVariableData& Variable, const char* Address = nullptr);

private:
	HazeVM* VM;
	void(*HookFunctionCall)(HazeVM* VM);
	void(*EndCall)();
	uint32 HookType;

	std::unordered_map<HAZE_STRING, std::pair<std::unordered_set<uint32>, HAZE_STRING>> HashMap_BreakPoint;
	std::unordered_map<HAZE_STRING, std::pair<std::unordered_set<uint32>, HAZE_STRING>> HashMap_TempBreakPoint;

	std::pair<HAZE_STRING, uint32> CurrPauseModule;
	bool IsPause;

	std::unordered_map<HAZE_STRING, bool> HashMap_IsStepOver;
};

