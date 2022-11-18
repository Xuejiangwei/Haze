#pragma once

//�ĸ��Ĵ��� AX(0),BX(1),CX(2),DX(3)

class HazeVM;

class OpCodeProcessor
{
public:
	OpCodeProcessor(HazeVM* VM);
	~OpCodeProcessor();

	int64_t GetNextBinary();

	HazeVM* GetVM() { return VM; }

private:
	HazeVM* VM;
};
