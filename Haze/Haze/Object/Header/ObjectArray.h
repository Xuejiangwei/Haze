#pragma once
#include "GCObject.h"

class HazeStack;
class HazeVM;

/*
* 定义时不能输入数字 例如 整数[1] 或 整数[甲] 这种形式是不允许的
*/

class ObjectArray : public GCObject
{
	friend class HazeMemory;
public:
	ObjectArray(x_uint32 gcIndex, HazeVM* vm, x_uint32 typeId, x_uint64* lengths);

	~ObjectArray();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	HazeVariableType GetType() const { return m_ValueType; }

	x_uint64 GetLength() const { return m_Length; }
	x_uint64 GetCapacity() const { return m_Capacity; }

	const char* GetIndex(x_uint64 index) const;

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
	HazeVariableType m_ValueType;
	//ClassData* m_ClassInfo;
};
