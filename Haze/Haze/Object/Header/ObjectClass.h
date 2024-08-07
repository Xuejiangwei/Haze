#pragma once
#include "ObjectDefine.h"
#include "HazeInstruction.h"

class ObjectClass
{
	friend class HazeMemory;
public:
	ObjectClass(ClassData* classInfo);

	~ObjectClass() {}

	void* GetOffset(uint64 offset) { return (char*)m_Data + offset; }

private:
	void* m_Data;
	ClassData* m_ClassInfo;
};
