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
	friend class ObjectClosure;
	friend class HazeDebugger;

	HazeStack(HazeVM* vm);

	~HazeStack();

public:
	char* GetAddressByEBP(int offset) { return &m_StackMain[(x_uint64)m_EBP + offset]; }
	char* GetAddressByESP(int offset) { return &m_StackMain[(x_uint64)m_ESP - offset]; }

	HazeVM* GetVM() const { return m_VM; }

	HAZE_ADDRESS_TYPE GetCurrPC() const { return m_PC; }

	void RunGlobalDataInit(x_int64 startPC, x_int64 endPC);

public:
	struct RegisterData
	{
		V_Array<char> Cmp_RegisterData;
	};

	struct HazeStackFrame
	{
		const FunctionData* FunctionInfo;
		x_uint32 FunctionParamSize;
		x_uint32 EBP;					// 之前调用函数栈的临时变量开始的地址
		x_uint32 ESP;					// 之前调用函数栈的临时变量结束的地址
		x_uint32 CurrParamESP;			// 当前函数的参数加PC地址的结束地址
		RegisterData Register;			// 比较寄存器的缓存

		HazeStackFrame(const FunctionData* Info, x_uint32 ParamSize, x_uint32 EBP, x_uint32 ESP, x_uint32 currParamESP, RegisterData& Register)
			: FunctionParamSize(ParamSize), EBP(EBP), ESP(ESP), CurrParamESP(currParamESP), Register(Register)
		{
			FunctionInfo = Info;
		}
	};

	const HazeStackFrame& GetCurrFrame() const { return m_StackFrame.back(); }

	bool FrameIsValid() { return m_StackFrame.size() > 0; }

	void OnError();

	void LogStack();

	void JmpTo(const InstructionData& m_Data);

	HazeRegister* GetVirtualRegister(HazeVirtualRegister type) { return &m_VirtualRegister[(x_uint8)type]; }

	const HazeVariableType& GetTempRegisterType(x_uint64 index) const;

	void ResetTempRegisterTypeByDynamicClassUnknow(x_uint64 index, const HazeVariableType& type);

private:
	void Run(bool isHazeCall = false);

	void PCStepInc();

	void InitStackRegister();

	void OnObjectFunctionCall();

	void OnObjectFunctionRet(int paramByteSize);

	void OnCall(const FunctionData* info, int paramSize);

	void OnRet();

	void ResetCallHaze();

	void AddCallHazeTimes();

	void SubCallHazeTimes();

	void PushGCTempRegister(void* address, const HazeVariableType* type);

	bool PopGCTempRegister(void* address);
	
private:
	HazeVM* m_VM;

	V_Array<char> m_StackMain;
	V_Array<HazeStackFrame> m_StackFrame;

	HAZE_ADDRESS_TYPE m_PC;

	x_uint32 m_EBP;		//栈底
	x_uint32 m_ESP;		//栈顶

	A_Array<HazeRegister, x_uint8(HazeVirtualRegister::_END)>  m_VirtualRegister;

	V_Array<int> m_CallHazeStack;

	V_Array<Pair<void*, const HazeVariableType*>> m_GCTempRegisters;

	V_Array<x_uint32> m_ObjectFunctionCallStack;
};