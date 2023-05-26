#pragma once

#include "Haze.h"

struct SourceLocation
{
	int64 Line;

	SourceLocation(int64 Line) : Line(Line)
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