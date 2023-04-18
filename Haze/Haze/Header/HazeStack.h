#pragma once

#include <vector>

#include "Haze.h"


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

		HazeStackFrame(const FunctionData* Info)
		{
			FunctionInfo = Info;
			Vector_LoopStack.clear();
		}
	};

	const HazeStackFrame& GetCurrFrame() const { return Stack_Frame.back(); }

	bool FrameIsValid() { return Stack_Frame.size(); }

	void JmpTo(const InstructionData& Data);

	void PushLoopStack();

	void PopLoopStack();

private:
	void PCStepInc();

	void PushMainFuntion();

	void InitRegisterToStack();

	void OnCall(const FunctionData* Info);

	void OnRet();

private:
	HazeVM* VM;

	std::vector<char> Stack_Main;
	std::vector<uint32> Stack_EBP;
	std::vector<HazeStackFrame> Stack_Frame;

	int PC;
	uint32 EBP;		//Õ»µ×
	uint32 ESP;		//Õ»¶¥
};