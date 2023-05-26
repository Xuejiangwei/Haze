#include "HazeDebugger.h"

HazeDebugger::HazeDebugger()
{
	HashSet_BreakPoint.clear();
}

HazeDebugger::~HazeDebugger()
{
}

void HazeDebugger::AddBreakPoint(const HAZE_STRING& FilePath, uint32 Line)
{
	HashSet_BreakPoint.insert({ FilePath, Line });
}

void HazeDebugger::StepOver()
{
}

void HazeDebugger::StepIn()
{
}
