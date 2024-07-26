#pragma once
#include "ObjectDefine.h"

class HazeStack;

class ObjectArray
{
	friend class HazeMemory;
public:
	ObjectArray(uint64 dimensionCount, void* address, uint64 length, uint64 pcAddress);

	~ObjectArray();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

private:
	static void GetLength(HazeStack* stack);

	static void GetLengthOfDimension(HazeStack* stack);

	static void GetDimensionCount(HazeStack* stack);

private:
	void* m_Data;
	uint64 m_DimensionCount;
	uint64 m_Length;
	uint64 m_PcAddress;
};
