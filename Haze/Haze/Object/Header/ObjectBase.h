#pragma once
#include "GCObject.h"

class HazeStack;

class ObjectBase : public GCObject
{
	friend class HazeMemory;
public:
	ObjectBase(x_uint32 gcIndex, HazeValueType type);

	~ObjectBase();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	static bool IsEqual(ObjectBase* obj1, ObjectBase* obj2);

private:
	static void Constructor(HAZE_OBJECT_CALL_PARAM);

	static void Equal(HAZE_OBJECT_CALL_PARAM);

	static void Get(HAZE_OBJECT_CALL_PARAM);

	static void Set(HAZE_OBJECT_CALL_PARAM);

private:
	HazeValueType m_Type;
	HazeValue m_Value;
};
