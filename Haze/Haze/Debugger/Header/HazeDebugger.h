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

	HazeDebugger(HazeVM* m_VM, void(*EndCall)());
	~HazeDebugger();

	enum DebuggerHookType
	{
		Line = 1,
		Instruction = 1 << 1,
		Function = 1 << 2,
	};

	void SetHook(void(*HookCall)(HazeVM* m_VM), uint32 m_Type);

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

	void SetJsonModuleGlobalVariable(open::OpenJson& Json);

public:
	void AddTempBreakPoint(uint32 Line);

	void SendProgramEnd();

private:
	void ClearCurrParseModuleData()
	{
		CurrPauseModule.first.clear();
		CurrPauseModule.second = 0;
		IsPause = false;
	}

	bool CurrModuleIsStepOver();

	void SetJsonType(open::OpenJson& Json, HazeDebugInfoType m_Type) { Json["Type"] = (int)m_Type; }

	void SetJsonBreakFilePath(open::OpenJson& Json, HAZE_STRING Path)
	{
		ReplacePathSlash(Path);
		auto m_Name = WString2String(Path);
		Json["BreakPathFile"] = GB2312_2_UFT8(m_Name.c_str());
	}

	void SetJsonBreakLine(open::OpenJson& Json, uint32 Line) { Json["BreakLine"] = Line; }

	void SendBreakInfo();

	void SetJsonVariableData(open::OpenJson& Json, const HazeVariableData& Variable, const char* Address = nullptr, bool isStack = true);

private:
	HazeVM* m_VM;
	void(*HookFunctionCall)(HazeVM* m_VM);
	void(*EndCall)();
	uint32 HookType;

	std::unordered_map<HAZE_STRING, std::pair<std::unordered_set<uint32>, HAZE_STRING>> HashMap_BreakPoint;
	std::unordered_map<HAZE_STRING, std::pair<std::unordered_set<uint32>, HAZE_STRING>> HashMap_TempBreakPoint;

	std::pair<HAZE_STRING, uint32> CurrPauseModule;
	bool IsPause;

	std::unordered_map<HAZE_STRING, bool> HashMap_IsStepOver;
};
