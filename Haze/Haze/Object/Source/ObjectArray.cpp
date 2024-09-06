#include "HazePch.h"
#include "ObjectArray.h"
#include "HazeMemory.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"

ObjectArray::ObjectArray(uint64 dimensionCount, uint64* lengths, uint64 pcAddress, HazeValueType valueType, ClassData* classInfo)
	: m_Data(nullptr), m_DimensionCount(dimensionCount), m_Length(0), m_PcAddress(pcAddress), m_ValueType(valueType),
	  m_ClassInfo(classInfo)
{
	if (dimensionCount > 1)
	{
		m_ValueType = HazeValueType::Array;
		m_Length = lengths[0];
		m_Capacity = m_Length;

		m_Data = HazeMemory::Alloca(m_Length * sizeof(ObjectArray));
		for (uint64 i = 0; i < m_Length; i++)
		{
			auto arr = HazeMemory::Alloca(sizeof(ObjectArray));
			new((char*)arr) ObjectArray(dimensionCount - 1, lengths + 1, pcAddress, valueType, classInfo);
			((ObjectArray**)m_Data)[i] = (ObjectArray*)arr;
		}
	}
	else if (lengths)
	{
		m_Length = lengths[0];
		m_Capacity = m_Length;

		if (m_Capacity == 0)
		{
			m_Capacity = 1;
			m_Data = HazeMemory::Alloca(m_Capacity * GetSizeByHazeType(valueType));
		}
		else
		{
			m_Data = HazeMemory::Alloca(m_Length * GetSizeByHazeType(valueType));
		}
	}
}

ObjectArray::~ObjectArray()
{
}

AdvanceClassInfo* ObjectArray::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	//info.Functions[H_TEXT("生成")] = { &ObjectArray::NewObjectArray, HazeValueType::Void, { HazeValueType::MultiVariable } };
	info.Functions[H_TEXT("长度")] = { &ObjectArray::GetLength, HazeValueType::UInt64, {} };
	info.Functions[H_TEXT("维之长度")] = { &ObjectArray::GetLengthOfDimension, HazeValueType::UInt64, { HazeValueType::UInt64 } };
	info.Functions[H_TEXT("维数")] = { &ObjectArray::GetDimensionCount, HazeValueType::UInt64, {} };
	info.Functions[H_TEXT("添加")] = { &ObjectArray::Add, HazeValueType::Void, { HazeValueType::MultiVariable } };

	info.Functions[HAZE_ADVANCE_GET_FUNCTION] = { &ObjectArray::Get, HazeValueType::Void, { HazeValueType::UInt64 } };
	info.Functions[HAZE_ADVANCE_SET_FUNCTION] = { &ObjectArray::Set, HazeValueType::Void, { HazeValueType::UInt64, HazeValueType::MultiVariable } };

	return &info;
}

//void ObjectArray::NewObjectArray(HAZE_STD_CALL_PARAM)
//{
//
//}

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
	ObjectArray* arr;
	char* value = nullptr;

	GET_PARAM_START();
	GET_PARAM(arr);

	auto size = GetSizeByHazeType(arr->m_ValueType);
	GET_PARAM_ADDRESS(value, size);

	if (arr->m_Length == arr->m_Capacity)
	{
		arr->m_Capacity *= 2;
		auto data = HazeMemory::Alloca(arr->m_Capacity * size);
		memcpy(data, arr->m_Data, arr->m_Length * size);
		HazeMemory::ManualFree(arr->m_Data);
		arr->m_Data = data;
	}

	memcpy((char*)arr->m_Data + arr->m_Length * size, value, size);
	arr->m_Length += 1;

	SET_RET_BY_TYPE(arr->m_ValueType, value);
}

void ObjectArray::Get(HAZE_STD_CALL_PARAM)
{
	ObjectArray* arr;
	uint64 offset = 0;

	GET_PARAM_START();
	GET_PARAM(arr);
	GET_PARAM(offset);

	auto size = GetSizeByHazeType(arr->m_ValueType);

	char value[8];
	memcpy(value, (char*)arr->m_Data + offset * size, size);
	SET_RET_BY_TYPE(arr->m_ValueType, value);
}

void ObjectArray::Set(HAZE_STD_CALL_PARAM)
{
	ObjectArray* arr;
	uint64 offset = 0;
	char* value = nullptr;

	GET_PARAM_START();
	GET_PARAM(arr);
	GET_PARAM(offset);

	auto size = GetSizeByHazeType(arr->m_ValueType);
	GET_PARAM_ADDRESS(value, size);

	memcpy((char*)arr->m_Data + offset * size, value, size);
}