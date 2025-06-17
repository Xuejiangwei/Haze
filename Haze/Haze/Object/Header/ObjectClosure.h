#pragma once
#include "GCObject.h"

class HazeStack;

/*
* 基本类型的值保留
*/

class ObjectClosure : public GCObject
{
	friend class HazeMemory;
public:
	ObjectClosure(x_uint32 gcIndex, HazeValueType keyType, ClassData* keyClassData, HazeValueType valueType, ClassData* valueClassData);

	~ObjectClosure();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	//static void NewObjectArray(HAZE_STD_CALL_PARAM);

private:
	static void GetLength(HAZE_OBJECT_CALL_PARAM);

	static void GetLengthOfDimension(HAZE_OBJECT_CALL_PARAM);

	static void GetDimensionCount(HAZE_OBJECT_CALL_PARAM);

	static void Add(HAZE_OBJECT_CALL_PARAM);

	static void Get(HAZE_OBJECT_CALL_PARAM);

	static void Set(HAZE_OBJECT_CALL_PARAM);

private:
	x_uint32 m_DataGCIndex;

	void* m_Data;
	x_uint64 m_Length;
	x_uint64 m_Capacity;
	HazeValueType m_KeyType;
	HazeValueType m_ValueType;
	ClassData* m_KeyClassInfo;
	ClassData* m_ValueClassInfo;
};
