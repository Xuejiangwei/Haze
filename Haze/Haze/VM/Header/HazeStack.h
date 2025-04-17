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
	char* GetAddressByEBP(int offset) { return &m_StackMain[(x_uint64)m_EBP + offset]; }
	char* GetAddressByESP(int offset) { return &m_StackMain[(x_uint64)m_ESP - offset]; }

	HazeVM* GetVM() const { return m_VM; }

	int GetCurrPC() const { return m_PC; }

	void RunGlobalDataInit(x_uint32 startPC, x_uint32 endPC);

public:
	struct RegisterData
	{
		V_Array<char> Cmp_RegisterData;
	};

	struct HazeStackFrame
	{
		const FunctionData* FunctionInfo;
		x_uint32 FunctionParamSize;
		x_uint32 EBP;
		x_uint32 ESP;
		RegisterData Register;

		HazeStackFrame(const FunctionData* Info, x_uint32 ParamSize, x_uint32 EBP, x_uint32 ESP, RegisterData& Register) 
			: FunctionParamSize(ParamSize), EBP(EBP), ESP(ESP), Register(Register)
		{
			FunctionInfo = Info;
		}
	};

	const HazeStackFrame& GetCurrFrame() const { return m_StackFrame.back(); }

	bool FrameIsValid() { return m_StackFrame.size() > 0; }

	void JmpTo(const InstructionData& m_Data);

	HazeRegister* GetVirtualRegister(const x_HChar* name) { return &m_VirtualRegister.find(name)->second; }

	const HazeDefineType& GetTempRegister(const x_HChar* name) const;

	void ResetTempRegisterTypeByDynamicClassUnknow(const HString& name, const HazeDefineType& type);

private:
	void Run(bool isHazeCall = false);

	void PCStepInc();

	void InitStackRegister();

	void OnCall(const FunctionData* info, int paramSize);

	void OnRet();

	void ResetCallHaze();

	void AddCallHazeTimes();

	void SubCallHazeTimes();

	void PushGCTempRegister(void* address, const HazeDefineType* type);

	bool PopGCTempRegister(void* address);

private:
	HazeVM* m_VM;

	V_Array<char> m_StackMain;
	V_Array<HazeStackFrame> m_StackFrame;

	int m_PC;
	x_uint32 m_EBP;		//Õ»µ×
	x_uint32 m_ESP;		//Õ»¶¥

	HashMap<HString, HazeRegister>  m_VirtualRegister;

	V_Array<int> m_CallHazeStack;

	V_Array<Pair<void*, const HazeDefineType*>> m_GCTempRegisters;
};