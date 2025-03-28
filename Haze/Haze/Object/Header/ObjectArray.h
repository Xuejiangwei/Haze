#pragma once
#include "ObjectDefine.h"

class HazeStack;

/*
* 定义时不能输入数字 例如 整数[1] 或 整数[甲] 这种形式是不允许的
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
