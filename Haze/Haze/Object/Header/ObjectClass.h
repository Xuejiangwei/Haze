#pragma once
#include "GCObject.h"
#include "HazeInstruction.h"

class ObjectClass : public GCObject
{
	friend class HazeVM;
	friend class HazeMemory;
	friend class InstructionProcessor;
public:
	ObjectClass(x_uint32 gcIndex, ClassData* classInfo);

	~ObjectClass();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	const char* GetMember(const x_HChar* memberName);

	void SetMember(const x_HChar* memberName, void* value);

private:
	static void GetOffset(HAZE_OBJECT_CALL_PARAM);

	static void SetOffset(HAZE_OBJECT_CALL_PARAM);

private:
	x_uint32 m_DataGCIndex;

	void* m_Data;
	ClassData* m_ClassInfo;
};
