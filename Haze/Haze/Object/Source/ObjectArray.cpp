#include "HazePch.h"
#include "ObjectArray.h"
#include "HazeMemory.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"
#include "ObjectHash.h"
#include "ObjectBase.h"
#include "ObjectClosure.h"

#define GET_OBJ(OBJ) CHECK_GET_STACK_OBJECT(OBJ, "����")

ObjectArray::ObjectArray(x_uint32 gcIndex, x_uint64 dimensionCount, x_uint64* lengths, x_uint64 pcAddress, HazeValueType valueType, ClassData* classInfo)
	: GCObject(gcIndex), m_Data(nullptr), m_DimensionCount(dimensionCount), m_Length(0), m_PcAddress(pcAddress), m_ValueType(valueType),
	  m_ClassInfo(classInfo)
{
	if (dimensionCount > 1)
	{
		m_ValueType.BaseType = HazeValueType::Array;
		m_Length = lengths[0];
		m_Capacity = m_Length;

		auto pair = HazeMemory::AllocaGCData(m_Length * sizeof(ObjectArray), GC_ObjectType::ArrayData);
		m_Data = pair.first;
		m_DataGCIndex = pair.second;

		for (x_uint64 i = 0; i < m_Length; i++)
		{
			pair = HazeMemory::HazeMemory::AllocaGCData(sizeof(ObjectArray), GC_ObjectType::Array);
			new((char*)pair.first) ObjectArray(pair.second, dimensionCount - 1, lengths + 1, pcAddress, valueType, classInfo);
			((ObjectArray**)m_Data)[i] = (ObjectArray*)pair.first;
		}
	}
	else if (lengths)
	{
		m_Length = lengths[0];
		m_Capacity = m_Length;

		if (m_Capacity == 0)
		{
			m_Capacity = 2;

			auto pair = HazeMemory::AllocaGCData(m_Capacity * GetSizeByHazeType(valueType), GC_ObjectType::ArrayData);
			m_Data = pair.first;
			m_DataGCIndex = pair.second;
		}
		else
		{
			auto pair = HazeMemory::AllocaGCData(m_Length * GetSizeByHazeType(valueType), GC_ObjectType::ArrayData);
			m_Data = pair.first;
			m_DataGCIndex = pair.second;
		}
	}
}

ObjectArray::~ObjectArray()
{
	if (m_DimensionCount <= 1)
	{
		HazeMemory::GetMemory()->Remove(m_Data, m_Capacity * GetSizeByHazeType(m_ValueType.BaseType), m_DataGCIndex);
	}
	else
	{
		HazeMemory::GetMemory()->Remove(m_Data, m_Capacity * sizeof(ObjectArray), m_DataGCIndex);
	}
}

AdvanceClassInfo* ObjectArray::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	//info.Functions[H_TEXT("����")] = { &ObjectArray::NewObjectArray, HazeValueType::Void, { HazeValueType::MultiVariable } };
	info.Add(H_TEXT("����"), { &ObjectArray::GetLength, OBJ_TYPE_DEF(UInt64), {} });
	info.Add(H_TEXT("ά֮����"), { &ObjectArray::GetLengthOfDimension, OBJ_TYPE_DEF(UInt64), { OBJ_TYPE_DEF(UInt64) } });
	info.Add(H_TEXT("ά��"), { &ObjectArray::GetDimensionCount, OBJ_TYPE_DEF(UInt64), {} });
	info.Add(H_TEXT("���"), { &ObjectArray::Add, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(MultiVariable) } });

	info.Add(HAZE_ADVANCE_GET_FUNCTION, { &ObjectArray::Get, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(UInt64) } });
	info.Add(HAZE_ADVANCE_SET_FUNCTION, { &ObjectArray::Set, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(UInt64), OBJ_TYPE_DEF(MultiVariable) } });

	return &info;
}

//void ObjectArray::NewObjectArray(HAZE_STD_CALL_PARAM)
//{
//
//}

void ObjectArray::GetLength(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_OBJ(arr);
	SET_RET_BY_TYPE(HazeValueType::UInt64, arr->m_Length);
}

void ObjectArray::GetLengthOfDimension(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	x_uint64 dimension;

	GET_PARAM_START();
	GET_OBJ(arr);
	GET_PARAM(dimension);
	SET_RET_BY_TYPE(HazeValueType::UInt64, stack->GetVM()->GetInstruction()[arr->m_PcAddress + dimension + 1].Operator[0].Extra.SignData);
}

void ObjectArray::GetDimensionCount(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_OBJ(arr);
	SET_RET_BY_TYPE(HazeValueType::UInt64, arr->m_DimensionCount);
}

void ObjectArray::Add(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	char* value = nullptr;

	GET_PARAM_START();
	GET_OBJ(arr);
	
	auto size = GetSizeByHazeType(arr->m_ValueType.BaseType);
	GET_PARAM_ADDRESS(value, size);

	if (arr->m_Length == arr->m_Capacity)
	{
		arr->m_Capacity *= 2;

		auto pair = HazeMemory::AllocaGCData(arr->m_Capacity * size, GC_ObjectType::ArrayData);
		memcpy(pair.first, arr->m_Data, arr->m_Length * size);

		arr->m_Data = pair.first;
		arr->m_DataGCIndex = pair.second;
	}

	memcpy((char*)arr->m_Data + arr->m_Length * size, value, size);
	arr->m_Length += 1;

	SET_RET_BY_TYPE(arr->m_ValueType.BaseType, value);
}

void ObjectArray::Get(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	x_uint64 offset = 0;

	GET_PARAM_START();
	GET_OBJ(arr);
	GET_PARAM(offset);

	auto size = GetSizeByHazeType(arr->m_ValueType.BaseType);

	char value[8];
	memcpy(value, (char*)arr->m_Data + offset * size, size);
	SET_RET_BY_TYPE(arr->m_ValueType.BaseType, value);
}

void ObjectArray::Set(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	x_uint64 offset = 0;
	char* value = nullptr;

	GET_PARAM_START();
	GET_OBJ(arr);
	GET_PARAM(offset);

	auto size = GetSizeByHazeType(arr->m_ValueType.BaseType);
	GET_PARAM_ADDRESS(value, size);

	memcpy((char*)arr->m_Data + offset * size, value, size);
}