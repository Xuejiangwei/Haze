#pragma once

class HazeVariable
{
public:
	friend class HazeExecuteFile;
	friend class HazeVM;
	friend class HazeMemory;

	HazeVariable();

	~HazeVariable();

	const HazeDefineType& GetType() const { return m_Type; }

	const HazeValue& GetValue() const { return Value; }

private:
	HString m_Name;

	HazeDefineType m_Type;

	union
	{
		HazeValue Value;
		void* Address;
	};
};
