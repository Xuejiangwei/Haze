#include "HazePch.h"
#include "ObjectArray.h"
#include "HazeMemory.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeCompiler.h"
#include "HazeLibraryDefine.h"

ObjectArray::ObjectArray(uint64 dimensionCount, void* address, uint64 length, uint64 pcAddress)
	: m_DimensionCount(dimensionCount), m_Data(address), m_Length(length), m_PcAddress(pcAddress)
{
}

ObjectArray::~ObjectArray()
{
}

AdvanceClassInfo* ObjectArray::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Functions[H_TEXT("长度")] = { &ObjectArray::GetLength, HazeValueType::UInt64 };
	info.Functions[H_TEXT("维之长度")] = { &ObjectArray::GetLengthOfDimension, HazeValueType::UInt64, { HazeValueType::UInt64 } };
	info.Functions[H_TEXT("维数")] = { &ObjectArray::GetDimensionCount, HazeValueType::UInt64 };
	return &info;
}

void ObjectArray::GetLength(HazeStack* stack)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_PARAM(arr);

	SET_RET_BY_TYPE(HazeValueType::UInt64, arr->m_Length);
}

void ObjectArray::GetLengthOfDimension(HazeStack* stack)
{
	ObjectArray* arr;
	uint64 dimension;

	GET_PARAM_START();
	GET_PARAM(arr);
	GET_PARAM(dimension);
	SET_RET_BY_TYPE(HazeValueType::UInt64, stack->GetVM()->GetInstruction()[arr->m_PcAddress + dimension + 1].Operator[0].Extra.SignData);
}

void ObjectArray::GetDimensionCount(HazeStack* stack)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_PARAM(arr);

	SET_RET_BY_TYPE(HazeValueType::UInt64, arr->m_DimensionCount);
}
