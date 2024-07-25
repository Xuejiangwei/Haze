#pragma once
#include "HazeDefine.h"

struct HazeRegister;
struct InstructionData;
struct FunctionData;
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
		V_Array<char> Cmp_RegisterData;
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

	HazeRegister* GetVirtualRegister(const HChar* name);

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

	//void RegisterArray(uint64 address, uint64 length);

	uint64 GetRegisterArrayLength(uint64 address);

	void GarbageCollection(bool force = false, bool collectionAll = false);

private:
	HazeVM* m_VM;

	V_Array<char> m_StackMain;
	V_Array<HazeStackFrame> m_StackFrame;

	int m_PC;
	uint32 m_EBP;		//Õ»µ×
	uint32 m_ESP;		//Õ»¶¥

	HashMap<HString, HazeRegister>  m_VirtualRegister;

	V_Array<int> m_CallHazeStack;
};