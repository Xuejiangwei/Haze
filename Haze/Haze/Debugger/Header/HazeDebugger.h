#pragma once

#include "HazeDebugInfo.h"
#include <unordered_set>

#include "XJson.h"

class HazeVM;

class HazeDebugger
{
public:
	friend class HazeVM;
	friend class HazeStack;

	HazeDebugger(HazeVM* vm, void(*endCall)());

	~HazeDebugger();

	enum class DebuggerHookType
	{
		Line = 1,
		Instruction = 1 << 1,
		Function = 1 << 2,
	};

	struct CurrParseModuleData
	{
		HAZE_STRING ModuleName;
		uint32 CurrLine;
	};

	void SetHook(void(*hookCall)(HazeVM* vm), uint32 type);

	void AddBreakPoint(const char* message);

	void DeleteBreakPoint(const char* message);

	void DeleteModuleAllBreakPoint(const char* message);

	void DeleteAllBreakPoint();

	void OnExecLine(uint32 line);

	void Start();

	void End();

	void StepOver();

	void StepIn();

	void StepInstruction();

	void Continue();

	void SetJsonLocalVariable(XJson& json);

	void SetJsonModuleGlobalVariable(XJson& json);

	bool IsPause() const { return m_IsPause; }

public:
	void AddTempBreakPoint(uint32 line);

	void AddTempBreakPoint(const HAZE_STRING& moduleName, uint32 line);

	void SendProgramEnd();

private:
	void ClearCurrParseModuleData()
	{
		m_CurrPauseModule.ModuleName.clear();
		m_CurrPauseModule.CurrLine = 0;
		m_IsPause = false;
		m_IsStepIn = false;
		m_IsStepInInstruction = false;
	}

	bool CurrModuleIsStepOver();

	void SetJsonType(XJson& json, HazeDebugInfoType type) { json["Type"] = (int)type; }

	void SetJsonBreakFilePath(XJson& json, HAZE_STRING path)
	{
		ReplacePathSlash(path);
		auto m_Name = WString2String(path);
		json["BreakPathFile"] = GB2312_2_UFT8(m_Name.c_str());
	}

	void SetJsonBreakLine(XJson& json, uint32 line) { json["BreakLine"] = line; }

	void SendBreakInfo();

	void SetJsonVariableData(XJson& json, const HazeVariableData& variable, const char* address = nullptr, bool isStack = true);

private:
	HazeVM* m_VM;
	void(*m_HookFunctionCall)(HazeVM* vm);
	void(*m_EndCall)();
	uint32 m_HookType;

	std::unordered_map<HAZE_STRING, std::pair<std::unordered_set<uint32>, HAZE_STRING>> m_BreakPoints;
	std::unordered_map<HAZE_STRING, std::pair<std::unordered_set<uint32>, HAZE_STRING>> m_TempBreakPoints;

	CurrParseModuleData m_CurrPauseModule;
	bool m_IsPause;

	std::vector<std::pair<HAZE_STRING, uint32>> m_StepInStack;
	std::unordered_map<HAZE_STRING, bool> m_IsStepOvers;

	bool m_IsStepIn;
	bool m_IsStepInInstruction;
};
