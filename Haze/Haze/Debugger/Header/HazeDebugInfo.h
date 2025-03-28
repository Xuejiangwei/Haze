#pragma once

enum class HazeDebugOperatorType : x_uint8
{
	None,
	Start,
	End,
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

enum class HazeDebugInfoType : x_uint8
{
	None,
	ProgramEnd,
	BreakInfo,
	StepInInfo,
};

struct SourceLocation
{
	x_uint32 Line;

	SourceLocation(x_uint32 Line) : Line(Line)
	{
	}
};

struct HookInfo
{
};

struct BreakPoint
{
	HString FilePath;
	x_uint32 Line;

	bool operator==(const BreakPoint& BP) const
	{
		return FilePath == BP.FilePath && Line == BP.Line;
	}
};

struct BreakPointHash
{
	x_uint64 operator()(const BreakPoint& BP) const
	{
		return std::hash<HString>()(BP.FilePath) + BP.Line;
	}
};