#pragma once
#include "GCObject.h"
#include "HazeInstruction.h"

class ObjectClass : public GCObject
{
	friend class HazeVM;
	friend class HazeMemory;
	friend class InstructionProcessor;
public:
	ObjectClass(x_uint32 gcIndex, HazeVM* vm, x_uint32 typeId);

	~ObjectClass();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	static bool IsEqual(ObjectClass* obj1, ObjectClass* obj2);

	static ObjectClass* Create(HazeVM* vm, ClassData* classData);

	const char* GetMember(const x_HChar* memberName);

	void SetMember(const x_HChar* memberName, void* value);

	const ClassData* GetClassData() const { return m_ClassInfo; }

private:
	static void GetOffset(HAZE_OBJECT_CALL_PARAM);

	static void SetOffset(HAZE_OBJECT_CALL_PARAM);

	static void GetClassName(HAZE_OBJECT_CALL_PARAM);

private:
	x_uint32 m_DataGCIndex;

	void* m_Data;
	ClassData* m_ClassInfo;
};
