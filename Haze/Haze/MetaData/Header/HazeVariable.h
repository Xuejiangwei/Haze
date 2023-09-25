#pragma once

#include "Haze.h"

class HazeVariable
{
public:
	friend class HazeExecuteFile;
	friend class HazeVM;
	friend class HazeMemory;

	HazeVariable();
	~HazeVariable();

	const HazeDefineType& GetType() const { return Type; }

	const HazeValue& GetValue() const { return m_Value; }

private:
	HAZE_STRING m_Name;

	HazeDefineType Type;

	union
	{
		HazeValue m_Value;
		void* Address;
	};
};
