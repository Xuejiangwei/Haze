#pragma once

#include <vector>

#include "Haze.h"

struct HazeRegister;

class MemoryPool;
class HazeVM;

class HazeStack
{
public:
	friend class InstructionProcessor;
	friend class GarbageCollection;

	HazeStack(HazeVM* VM);

	~HazeStack();

	void Start(unsigned int Address);

public:
	char* GetAddressByEBP(int Offset) { return &Stack_Main[(uint64)EBP + Offset]; }

	HazeVM* GetVM() const { return VM; }

	int GetCurrPC() const { return PC; }

public:
	struct HazeStackFrame
	{
		const FunctionData* FunctionInfo;
		uint32 FunctionParamSize;
		uint32 EBP;
		uint32 ESP;

		HazeStackFrame(const FunctionData* Info, uint32 ParamSize, uint32 EBP, uint32 ESP) : FunctionParamSize(ParamSize), EBP(EBP), ESP(ESP)
		{
			FunctionInfo = Info;
		}
	};

	const HazeStackFrame& GetCurrFrame() const { return Stack_Frame.back(); }

	bool FrameIsValid() { return Stack_Frame.size(); }

	void JmpTo(const InstructionData& Data);

	HazeRegister* GetVirtualRegister(const HAZE_CHAR* Name);

private:
	void Run(bool IsHazeCall = false);

	void PCStepInc();

	void PreMainFunction();

	void PushMainFuntion();

	void InitStackRegister();

	void OnCall(const FunctionData* Info, int ParamSize);

	void OnRet();

	void ResetCallHaze();

private:
	void* Alloca(uint32 Size);

	void GarbageCollection(bool Force = false, bool CollectionAll = false);

private:
	HazeVM* VM;

	std::vector<char> Stack_Main;
	std::vector<HazeStackFrame> Stack_Frame;

	int PC;
	uint32 EBP;		//ջ��
	uint32 ESP;		//ջ��

	std::unordered_map<HAZE_STRING, HazeRegister>  HashMap_VirtualRegister;

	std::vector<int> Vector_CallHazeStack;
};