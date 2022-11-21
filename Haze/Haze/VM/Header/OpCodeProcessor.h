#pragma once

//ËÄ¸ö¼Ä´æÆ÷ AX(0),BX(1),CX(2),DX(3)

class HazeVM;

class OpCodeProcessor
{
public:
	OpCodeProcessor(HazeVM* VM);
	~OpCodeProcessor();

	int64_t GetNextBinary() { return 0; }

	HazeVM* GetVM() { return VM; }

private:
	HazeVM* VM;
};
