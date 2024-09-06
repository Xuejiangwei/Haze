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
	ObjectArray(uint64 dimensionCount, uint64* length, uint64 pcAddress, HazeValueType valueType, ClassData* classInfo = nullptr);

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
	uint64 m_DimensionCount;
	uint64 m_Length;
	uint64 m_Capacity;
	uint64 m_PcAddress;
	HazeValueType m_ValueType;
	ClassData* m_ClassInfo;
};
