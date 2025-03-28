#pragma once
#include "ObjectDefine.h"
#include "HazeInstruction.h"

class ObjectString
{
public:
	ObjectString(const x_HChar* str, bool fixedCapacity = false);

	~ObjectString();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	const x_HChar* GetData() const { return (x_HChar*)m_Data; }

	x_uint64 GetLength() const { return m_Length; }

private:
	static void Append(HAZE_STD_CALL_PARAM);

	static void Format(HAZE_STD_CALL_PARAM);

private:
	void* m_Data;
	x_uint64 m_Length;
	x_uint64 m_Capacity;
};