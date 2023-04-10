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
		HAZE_STRING FunctionName;
		std::vector<HazeJmpData> Stack_Jmp;
	};

	HazeStackFrame& GetCurrFrame() { return Stack_Frame.back(); }

	bool FrameIsValid() { return Stack_Frame.size(); }

	void PushJmpStack(const HazeJmpData& Data);

private:
	void PCStepInc();

	void PushMainFuntion();

	void InitRegisterToStack();

	void OnCall(const HAZE_STRING& Name);

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