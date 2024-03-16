#pragma once

#include "HazeHeader.h"

enum class HazeDebugOperatorType : uint8
{
	None,
	Start,
	StepOver,
	StepIn,
	StepInstruction,

	AddBreakPoint,
	DeleteBreakPoint,
	DeleteModuleAllBreakPoint,
	DeleteAllBreakPoint,

	Continue,

	GetLocalVariable,
};

enum class HazeDebugInfoType : uint8
{
	None,
	ProgramEnd,
	BreakInfo,
};

struct SourceLocation
{
	uint32 Line;

	SourceLocation(uint32 Line) : Line(Line)
	{
	}
};

struct HookInfo
{
};

struct BreakPoint
{
	HAZE_STRING FilePath;
	uint32 Line;

	bool operator==(const BreakPoint& BP) const
	{
		return FilePath == BP.FilePath && Line == BP.Line;
	}
};

struct BreakPointHash
{
	uint64 operator()(const BreakPoint& BP) const
	{
		return std::hash<HAZE_STRING>()(BP.FilePath) + BP.Line;
	}
};