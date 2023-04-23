#pragma once

#include <vector>

#include "Haze.h"

class MemoryPool;
class HazeVM;

class HazeStack
{
public:
	friend class InstructionProcessor;

	HazeStack(HazeVM* VM);

	~HazeStack();

	void Start(unsigned int Address);

public:
	char* GetAddressByEBP(int Offset) { return &Stack_Main[EBP + Offset]; }

	const HazeVM* GetVM() const { return VM; }

public:
	struct HazeStackFrame
	{
		const FunctionData* FunctionInfo;
		std::vector<int> Vector_LoopStack;
		uint32 FunctionParamSize;

		HazeStackFrame(const FunctionData* Info, uint32 ParamSize)
		{
			FunctionInfo = Info;
			Vector_LoopStack.clear();
			FunctionParamSize = ParamSize;
		}
	};

	const HazeStackFrame& GetCurrFrame() const { return Stack_Frame.back(); }

	bool FrameIsValid() { return Stack_Frame.size(); }

	void JmpTo(const InstructionData& Data);

	void PushLoopStack();

	void PopLoopStack();

private:
	void PCStepInc();

	void PreMainFunction();

	void PushMainFuntion();

	void InitRegisterToStack();

	void OnCall(const FunctionData* Info, int ParamSize);

	void OnRet();

	void* Alloca(unsigned int Size);

	void GarbageCollection(bool Force = false, bool CollectionAll = false);

private:
	HazeVM* VM;

	std::vector<char> Stack_Main;
	std::vector<uint32> Stack_EBP;
	std::vector<HazeStackFrame> Stack_Frame;

	int PC;
	uint32 EBP;		//Õ»µ×
	uint32 ESP;		//Õ»¶¥

	std::vector<std::unique_ptr<MemoryPool>> Vector_MemoryPool;
};