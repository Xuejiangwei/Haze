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

	HazeDebugger(HazeVM* vm, void(*endCall)());

	~HazeDebugger();

	enum DebuggerHookType
	{
		Line = 1,
		Instruction = 1 << 1,
		Function = 1 << 2,
	};

	void SetHook(void(*hookCall)(HazeVM* vm), uint32 type);

	void AddBreakPoint(const char* message);

	void DeleteBreakPoint(const char* message);

	void DeleteModuleAllBreakPoint(const char* message);

	void DeleteAllBreakPoint();

	void OnExecLine(uint32 line);

	void Start();

	void StepOver();

	void StepIn();

	void StepInstruction();

	void Continue();

	void SetJsonLocalVariable(open::OpenJson& json);

	void SetJsonModuleGlobalVariable(open::OpenJson& json);

	bool IsPause() const { return m_IsPause; }

public:
	void AddTempBreakPoint(uint32 line);

	void SendProgramEnd();

private:
	void ClearCurrParseModuleData()
	{
		m_CurrPauseModule.first.clear();
		m_CurrPauseModule.second = 0;
		m_IsPause = false;
	}

	bool CurrModuleIsStepOver();

	void SetJsonType(open::OpenJson& json, HazeDebugInfoType type) { json["Type"] = (int)type; }

	void SetJsonBreakFilePath(open::OpenJson& json, HAZE_STRING path)
	{
		ReplacePathSlash(path);
		auto m_Name = WString2String(path);
		json["BreakPathFile"] = GB2312_2_UFT8(m_Name.c_str());
	}

	void SetJsonBreakLine(open::OpenJson& json, uint32 line) { json["BreakLine"] = line; }

	void SendBreakInfo();

	void SetJsonVariableData(open::OpenJson& json, const HazeVariableData& variable, const char* address = nullptr, bool isStack = true);

private:
	HazeVM* m_VM;
	void(*m_HookFunctionCall)(HazeVM* vm);
	void(*m_EndCall)();
	uint32 m_HookType;

	std::unordered_map<HAZE_STRING, std::pair<std::unordered_set<uint32>, HAZE_STRING>> m_BreakPoints;
	std::unordered_map<HAZE_STRING, std::pair<std::unordered_set<uint32>, HAZE_STRING>> m_TempBreakPoints;

	std::pair<HAZE_STRING, uint32> m_CurrPauseModule;
	bool m_IsPause;

	std::unordered_map<HAZE_STRING, bool> m_IsStepOvers;
};
