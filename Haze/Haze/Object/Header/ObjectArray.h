#pragma once
#include "GCObject.h"

class HazeStack;

/*
* ����ʱ������������ ���� ����[1] �� ����[��] ������ʽ�ǲ������
*/

class ObjectArray : public GCObject
{
	friend class HazeMemory;
public:
	ObjectArray(x_uint32 gcIndex, x_uint64 dimensionCount, x_uint64* length, x_uint64 pcAddress, HazeValueType valueType, ClassData* classInfo = nullptr);

	~ObjectArray();

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
	x_uint64 m_DimensionCount;
	x_uint64 m_Length;
	x_uint64 m_Capacity;
	x_uint64 m_PcAddress;
	HazeValueType m_ValueType;
	ClassData* m_ClassInfo;
};
