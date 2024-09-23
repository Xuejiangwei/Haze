#pragma once
#include "ObjectDefine.h"
#include "HazeInstruction.h"

class ObjectClass
{
	friend class HazeMemory;
	friend class InstructionProcessor;
public:
	ObjectClass(ClassData* classInfo);

	~ObjectClass() {}

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

private:
	static void GetOffset(HAZE_STD_CALL_PARAM);

	static void SetOffset(HAZE_STD_CALL_PARAM);

private:
	void* m_Data;
	ClassData* m_ClassInfo;
};
