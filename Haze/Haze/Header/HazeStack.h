#pragma once

#include <vector>

#include "Haze.h"


class HazeFrameStack
{
};

class HazeStack
{
public:
	HazeStack();

	~HazeStack();

	HazeValue* GetVirtualRegister(const HAZE_CHAR* Name);

	void Call();

	void Push(const std::vector<InstructionData>& Operator);

	void Pop();

	void Add(const std::vector<InstructionData>& Operator);

private:
	void InitRegisterToStack();

	HazeValue* GetInstructionValue(const InstructionData& InsData);

private:
	std::vector<char> Stack_Main;
	std::vector<HazeValueType> Stack_Type; // ÔÝÊ±²»ÐèÒª¸¨ÖúÕ»

	unsigned int PC;
	unsigned int EBP;		//Õ»µ×
	unsigned int ESP;		//Õ»¶¥
};