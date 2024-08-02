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


	// �������ַ����͵ľ�̬����
	static void Format(HAZE_STD_CALL_PARAM);

private:
	void* m_Data;
	uint64 m_Length;
	uint64 m_Capacity;
};