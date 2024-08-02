#pragma once
#include "ObjectDefine.h"
#include "HazeInstruction.h"

class ObjectString : public ObjectBase
{
public:
	ObjectString(const HChar* str, bool fixedCapacity = false);

	~ObjectString();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	const HChar* GetData() const { return (HChar*)m_Data; }

	uint64 GetLength() const { return m_Length; }

private:
	static void Append(class HazeStack* stack);


	// 以下是字符类型的静态函数
	static void Format(HAZE_STD_CALL_PARAM);

private:
	void* m_Data;
	uint64 m_Length;
	uint64 m_Capacity;
};