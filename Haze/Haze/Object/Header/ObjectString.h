#pragma once
#include "ObjectDefine.h"
#include "HazeInstruction.h"

class ObjectString : public ObjectBase
{
public:
	ObjectString(const HChar* str, bool fixedCapacity = false);

	~ObjectString();

	//Haze可调用方法
	uint64 GetLength() const { return m_Length; }

	void Append(const HChar* str);

private:
	void* m_Data;
	uint64 m_Length;
	uint64 m_Capacity;
};