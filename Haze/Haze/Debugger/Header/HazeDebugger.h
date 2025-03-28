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
		HString ModuleName;
		x_uint32 CurrLine;
	};

	void SetHook(void(*hookCall)(HazeVM* vm), x_uint32 type);

	void AddBreakPoint(const char* message);

	void DeleteBreakPoint(const char* message);

	void DeleteModuleAllBreakPoint(const char* message);

	void DeleteAllBreakPoint();

	void OnExecLine(x_uint32 line);

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
	void AddTempBreakPoint(x_uint32 line);

	void AddTempBreakPoint(const HString& moduleName, x_uint32 line);

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

	void SetJsonBreakFilePath(XJson& json, HString path)
	{
		ReplacePathSlash(path);
		auto m_Name = WString2String(path);
		json["BreakPathFile"] = GB2312_2_UFT8(m_Name.c_str());
	}

	void SetJsonBreakLine(XJson& json, x_uint32 line) { json["BreakLine"] = line; }

	void SendBreakInfo();

	void SetJsonVariableData(XJson& json, const HazeVariableData& variable, const char* address = nullptr, bool isStack = true);

private:
	HazeVM* m_VM;
	void(*m_HookFunctionCall)(HazeVM* vm);
	void(*m_EndCall)();
	x_uint32 m_HookType;

	HashMap<HString, Pair<HashSet<x_uint32>, HString>> m_BreakPoints;
	HashMap<HString, Pair<HashSet<x_uint32>, HString>> m_TempBreakPoints;

	CurrParseModuleData m_CurrPauseModule;
	bool m_IsPause;

	V_Array<Pair<HString, x_uint32>> m_StepInStack;
	HashMap<HString, bool> m_IsStepOvers;

	bool m_IsStepIn;
	bool m_IsStepInInstruction;
};
