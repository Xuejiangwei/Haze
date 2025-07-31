#pragma once
#include "GCObject.h"
#include "HazeInstruction.h"

class ObjectString : public GCObject
{
	friend class HazeMemory;
public:
	ObjectString(x_uint32 gcIndex, const x_HChar* str, bool fixedCapacity = false);

	~ObjectString();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	static bool IsEqual(ObjectString* obj1, ObjectString* obj2);

	static ObjectString* Create(const x_HChar* str = nullptr, bool fixedCapacity = false);

	const x_HChar* GetData() const { return (x_HChar*)m_Data; }

	x_uint64 GetLength() const { return m_Length; }

	x_uint64 Hash() const;

private:
	static void Append(HAZE_OBJECT_CALL_PARAM);

	static void Format(HAZE_OBJECT_CALL_PARAM);

private:
	x_uint32 m_DataGCIndex;
	
	void* m_Data;
	x_uint64 m_Length;
	x_uint64 m_Capacity;
};