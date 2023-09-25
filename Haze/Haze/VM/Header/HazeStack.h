#pragma once

#include <vector>

#include "Haze.h"

struct HazeRegister;

class HazeVM;

class HazeStack
{
public:
	friend class InstructionProcessor;
	friend class HazeMemory;

	HazeStack(HazeVM* m_VM);

	~HazeStack();

	void Start(unsigned int Address);

public:
	char* GetAddressByEBP(int Offset) { return &Stack_Main[(uint64)EBP + Offset]; }

	HazeVM* GetVM() const { return m_VM; }

	int GetCurrPC() const { return PC; }

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

	const HazeStackFrame& GetCurrFrame() const { return Stack_Frame.back(); }

	bool FrameIsValid() { return Stack_Frame.size(); }

	void JmpTo(const InstructionData& m_Data);

	HazeRegister* GetVirtualRegister(const HAZE_CHAR* m_Name);

private:
	void Run(bool IsHazeCall = false);

	void PCStepInc();

	void PreMainFunction();

	void PushMainFuntion();

	void InitStackRegister();

	void OnCall(const FunctionData* Info, int ParamSize);

	void OnRet();

	void ResetCallHaze();

	void AddCallHazeTimes();

	void SubCallHazeTimes();

private:
	void* Alloca(uint32 Size);

	void GarbageCollection(bool Force = false, bool CollectionAll = false);

private:
	HazeVM* m_VM;

	std::vector<char> Stack_Main;
	std::vector<HazeStackFrame> Stack_Frame;

	int PC;
	uint32 EBP;		//Õ»µ×
	uint32 ESP;		//Õ»¶¥

	std::unordered_map<HAZE_STRING, HazeRegister>  HashMap_VirtualRegister;

	std::vector<int> Vector_CallHazeStack;
};