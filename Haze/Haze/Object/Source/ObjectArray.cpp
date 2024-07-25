#include "HazePch.h"
#include "ObjectArray.h"
#include "HazeMemory.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeCompiler.h"
#include "HazeLibraryDefine.h"

ObjectArray::ObjectArray(uint64* dimensions, uint64 dimensionCount, uint64 size)
	: m_Dimensions(dimensions), m_DimensionCount(dimensionCount)
{
	m_Data = HazeMemory::Alloca(size);
}

ObjectArray::~ObjectArray()
{
	delete m_Dimensions;

	//auto ff = &ObjectArray::GetLength;
	//(this->*ff)();
}

AdvanceClassInfo* ObjectArray::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Functions[H_TEXT("³¤¶È")] = { &ObjectArray::GetLength, {} };
	return &info;
}

void ObjectArray::GetLength(HazeStack* stack)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_PARAM(arr);

	uint64 length = 1;
	for (uint64 i = 0; i < arr->m_DimensionCount; i++)
	{
		length *= *(arr->m_Dimensions + i);
	}

	SET_RET_BY_TYPE(HazeValueType::UInt64, length);
}