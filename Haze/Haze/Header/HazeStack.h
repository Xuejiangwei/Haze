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

private:
	void PushMainFuntion();

	void InitRegisterToStack();

private:
	HazeVM* VM;

	std::vector<char> Stack_Main;
	std::vector<uint> Stack_EBP;
	std::vector<HAZE_STRING> Stack_Function; // ��ʱ����Ҫ����ջ

	int PC;
	uint EBP;		//ջ��
	uint ESP;		//ջ��
};