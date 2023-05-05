#pragma once

#include "Haze.h"

class HazeVariable
{
public:
	friend class HazeVM;
	friend class GarbageCollection;

	HazeVariable();
	~HazeVariable();

	const HazeDefineType& GetType() const { return Type; }

	const HazeValue& GetValue() const { return Value; }

private:
	HAZE_STRING Name;

	HazeDefineType Type;
	HazeValue Value;
};
