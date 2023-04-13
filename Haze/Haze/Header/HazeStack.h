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
		FunctionData* FunctionInfo;
	};

	HazeStackFrame& GetCurrFrame() { return Stack_Frame.back(); }

	bool FrameIsValid() { return Stack_Frame.size(); }

	/*void PushVariableStack(HazeDefineVariable* Variable);

	void PopVariableStack(int Num = 1);

	void PushJmpStack(const InstructionData& Data, bool IsPush = true);

	void PopCurrJmpStack();*/

private:
	void PCStepInc();

	void PushMainFuntion();

	void InitRegisterToStack();

	void OnCall(FunctionData* Info);

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