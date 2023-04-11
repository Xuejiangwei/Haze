#pragma once

#include "HazeDefine.h"

enum class HazeToken : uint32;
enum class InstructionOpCode : uint32;

enum class HazeCmpType : uint32
{
	None,
	Equal,
	NotEqual,
	Greater,
	GreaterEqual,
	Less,
	LessEqual,
};

HazeCmpType GetHazeCmpTypeByToken(HazeToken Token);

const HAZE_CHAR* GetInstructionStringByCmpType(HazeCmpType Type);