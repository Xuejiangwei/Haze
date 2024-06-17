#pragma once

#include <vector>

#include "HazeHeader.h"

struct HazeRegister;

class HazeVM;

class HazeStack
{
public:
	friend class InstructionProcessor;
	friend class HazeMemory;

	HazeStack(HazeVM* vm);

	~HazeStack();

public:
	char* GetAddressByEBP(int offset) { return &m_StackMain[(uint64)m_EBP + offset]; }

	char* GetAddressByESP(int offset) { return &m_StackMain[(uint64)m_ESP - offset]; }

	HazeVM* GetVM() const { return m_VM; }

	int GetCurrPC() const { return m_PC; }

	void RunGlobalDataInit(uint32 startPC, uint32 endPC);

public:
	struct RegisterData
	{
		std::vector<char> Cmp_RegisterData;
	};

	struct HazeStackFrame
	{
		const FunctionData* FunctionInfo;
		uint32 FunctionParamSize;
		uint32 EBP;
		uint32 ESP;
		RegisterData Register;

		HazeStackFrame(const FunctionData* Info, uint32 ParamSize, uint32 EBP, uint32 ESP, RegisterData& Register) 
			: FunctionParamSize(ParamSize), EBP(EBP), ESP(ESP), Register(Register)
		{
			FunctionInfo = Info;
		}
	};

	const HazeStackFrame& GetCurrFrame() const { return m_StackFrame.back(); }

	bool FrameIsValid() { return m_StackFrame.size(); }

	void JmpTo(const InstructionData& m_Data);

	HazeRegister* GetVirtualRegister(const HAZE_CHAR* name);

private:
	void Run(bool isHazeCall = false);

	void PCStepInc();

	void InitStackRegister();

	void OnCall(const FunctionData* info, int paramSize);

	void OnRet();

	void ResetCallHaze();

	void AddCallHazeTimes();

	void SubCallHazeTimes();

private:
	void* Alloca(uint64 size);

	void RegisterArray(uint64 address, uint64 length);

	uint64 GetRegisterArrayLength(uint64 address);

	void GarbageCollection(bool force = false, bool collectionAll = false);

private:
	HazeVM* m_VM;

	std::vector<char> m_StackMain;
	std::vector<HazeStackFrame> m_StackFrame;

	int m_PC;
	uint32 m_EBP;		//Õ»µ×
	uint32 m_ESP;		//Õ»¶¥

	std::unordered_map<HAZE_STRING, HazeRegister>  m_VirtualRegister;

	std::vector<int> m_CallHazeStack;
};