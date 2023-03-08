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

private:
	void InitRegisterToStack();

private:
	HazeVM* VM;

	std::vector<char> Stack_Main;
	std::vector<char> Stack_EBP;
	std::vector<HAZE_STRING> Stack_Function; // ��ʱ����Ҫ����ջ

	int PC;
	unsigned int EBP;		//ջ��
	unsigned int ESP;		//ջ��
};