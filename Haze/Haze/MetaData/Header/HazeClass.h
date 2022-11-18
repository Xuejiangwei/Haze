#pragma once

#include <vector>

class HazeVariable;
class HazeFunction;

class HazeClass
{
public:
	HazeClass();
	~HazeClass();

	void AddMemberData();
	void AddMemberFunction();

private:
	std::vector<HazeVariable> Variables;
	std::vector<HazeFunction> Functions;
};
