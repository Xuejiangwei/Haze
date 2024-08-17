#include "HazePch.h"
#include "ObjectArray.h"
#include "HazeMemory.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeCompiler.h"
#include "HazeLibraryDefine.h"

ObjectArray::ObjectArray(uint64 dimensionCount, void* address, uint64 length, uint64 pcAddress, HazeValueType valueType, ClassData* classInfo)
	: m_DimensionCount(dimensionCount), m_Data(address), m_Length(length), m_PcAddress(pcAddress), m_ValueType(valueType),
	  m_ClassInfo(classInfo)
{
}

ObjectArray::~ObjectArray()
{
}

AdvanceClassInfo* ObjectArray::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Functions[H_TEXT("长度")] = { AdvanceFunctionType::ObjectFunction, &ObjectArray::GetLength, HazeValueType::UInt64, {} };
	info.Functions[H_TEXT("维之长度")] = { AdvanceFunctionType::ObjectFunction, &ObjectArray::GetLengthOfDimension,
		HazeValueType::UInt64, { HazeValueType::UInt64 } };
	info.Functions[H_TEXT("维数")] = { AdvanceFunctionType::ObjectFunction, &ObjectArray::GetDimensionCount,
		HazeValueType::UInt64, {} };
	info.Functions[H_TEXT("添加")] = { AdvanceFunctionType::ObjectFunction, &ObjectArray::Add, HazeValueType::Void,
		{ HazeValueType::MultiVariable } };

	return &info;
}

void ObjectArray::GetLength(HAZE_STD_CALL_PARAM)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_PARAM(arr);

	SET_RET_BY_TYPE(HazeValueType::UInt64, arr->m_Length);
}

void ObjectArray::GetLengthOfDimension(HAZE_STD_CALL_PARAM)
{
	ObjectArray* arr;
	uint64 dimension;

	GET_PARAM_START();
	GET_PARAM(arr);
	GET_PARAM(dimension);
	SET_RET_BY_TYPE(HazeValueType::UInt64, stack->GetVM()->GetInstruction()[arr->m_PcAddress + dimension + 1].Operator[0].Extra.SignData);
}

void ObjectArray::GetDimensionCount(HAZE_STD_CALL_PARAM)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_PARAM(arr);

	SET_RET_BY_TYPE(HazeValueType::UInt64, arr->m_DimensionCount);
}

void ObjectArray::Add(HAZE_STD_CALL_PARAM)
{
}

void ObjectArray::Get(HAZE_STD_CALL_PARAM)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_PARAM(arr);

	/*for (uint64 i = 0; i < multiParamNum; i++)
	{
		arr->m_Data + 
	}*/
}
