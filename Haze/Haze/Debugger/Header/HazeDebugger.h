#pragma once

#include "HazeDebugInfo.h"
#include <unordered_set>

class HazeDebugger
{
public:
	HazeDebugger();
	~HazeDebugger();

	void AddBreakPoint(const HAZE_STRING& FilePath, uint32 Line);

	void StepOver();

	void StepIn();

private:
	std::unordered_set<BreakPoint, BreakPointHash> HashSet_BreakPoint;
};

