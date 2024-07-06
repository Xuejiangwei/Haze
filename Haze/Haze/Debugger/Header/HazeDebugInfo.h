#pragma once

enum class HazeDebugOperatorType : uint8
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

enum class HazeDebugInfoType : uint8
{
	None,
	ProgramEnd,
	BreakInfo,
	StepInInfo,
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
	HString FilePath;
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
		return std::hash<HString>()(BP.FilePath) + BP.Line;
	}
};