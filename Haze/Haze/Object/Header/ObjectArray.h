#pragma once
#include "ObjectDefine.h"

class HazeStack;

class ObjectArray
{
public:
	ObjectArray(uint64* dimensions, uint64 dimensionCount, uint64 size);

	~ObjectArray();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

private:
	static void GetLength(HazeStack* stack);

private:
	void* m_Data;
	uint64* m_Dimensions;
	uint64 m_DimensionCount;
};
