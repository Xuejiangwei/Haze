#pragma once
#include "ObjectDefine.h"

class HazeStack;

/*
* ����ʱ������������ ���� ����[1] �� ����[��] ������ʽ�ǲ������
*/

class ObjectArray
{
	friend class HazeMemory;
public:
	ObjectArray(x_uint64 dimensionCount, x_uint64* length, x_uint64 pcAddress, HazeValueType valueType, ClassData* classInfo = nullptr);

	~ObjectArray();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	//static void NewObjectArray(HAZE_STD_CALL_PARAM);

private:
	static void GetLength(HAZE_STD_CALL_PARAM);

	static void GetLengthOfDimension(HAZE_STD_CALL_PARAM);

	static void GetDimensionCount(HAZE_STD_CALL_PARAM);

	static void Add(HAZE_STD_CALL_PARAM);

	static void Get(HAZE_STD_CALL_PARAM);

	static void Set(HAZE_STD_CALL_PARAM);

private:
	void* m_Data;
	x_uint64 m_DimensionCount;
	x_uint64 m_Length;
	x_uint64 m_Capacity;
	x_uint64 m_PcAddress;
	HazeValueType m_ValueType;
	ClassData* m_ClassInfo;
};
