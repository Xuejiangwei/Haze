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

#define DEFAULT_CAPACITY 2
#define GET_OBJ(OBJ) CHECK_GET_STACK_OBJECT(OBJ, "数组")

ObjectArray::ObjectArray(x_uint32 gcIndex, HazeVM* vm, x_uint32 typeId, x_uint64* lengths)
	: GCObject(gcIndex)
{
	auto info = vm->GetTypeInfoMap()->GetTypeById(typeId);

	m_ValueType.BaseType = vm->GetTypeInfoMap()->GetTypeById(info->_Array.TypeId1)->GetBaseType();
	m_ValueType.TypeId = info->_Array.TypeId1;
	m_DimensionCount = info->_Array.Dimension;
	if (info->_Array.Dimension > 1)
	{
		m_Length = lengths[0];
		m_Capacity = m_Length;

		auto pair = HAZE_MALLOC(m_Length * sizeof(ObjectArray), GC_ObjectType::ArrayData);
		m_Data = pair.first;
		m_DataGCIndex = pair.second;

		for (x_uint64 i = 0; i < m_Length; i++)
		{
			pair = HAZE_MALLOC(sizeof(ObjectArray), GC_ObjectType::Array);

			new((char*)pair.first) ObjectArray(pair.second, vm, m_ValueType.TypeId, lengths + 1);
			((ObjectArray**)m_Data)[i] = (ObjectArray*)pair.first;
		}
	}
	else if (lengths)
	{
		m_Length = lengths[0];
		m_Capacity = m_Length;

		if (m_Capacity == 0)
		{
			m_Capacity = DEFAULT_CAPACITY;

			auto pair = HAZE_MALLOC(m_Capacity * GetSizeByHazeType(m_ValueType.BaseType), GC_ObjectType::ArrayData);
			m_Data = pair.first;
			m_DataGCIndex = pair.second;
		}
		else
		{
			auto pair = HAZE_MALLOC(m_Length * GetSizeByHazeType(m_ValueType.BaseType), GC_ObjectType::ArrayData);
			m_Data = pair.first;
			m_DataGCIndex = pair.second;
		}
	}
}

ObjectArray::~ObjectArray()
{
	if (m_DimensionCount <= 1)
	{
		HAZE_FREE(m_Data, m_Capacity * GetSizeByHazeType(m_ValueType.BaseType), m_DataGCIndex);
	}
	else
	{
		HAZE_FREE(m_Data, m_Capacity * sizeof(ObjectArray), m_DataGCIndex);
	}
}

AdvanceClassInfo* ObjectArray::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	//info.Functions[H_TEXT("生成")] = { &ObjectArray::NewObjectArray, HazeValueType::Void, { HazeValueType::MultiVariable } };
	info.Add(H_TEXT("长度"), { &ObjectArray::GetLength, OBJ_TYPE_DEF(UInt64), {} });
	info.Add(H_TEXT("维之长度"), { &ObjectArray::GetLengthOfDimension, OBJ_TYPE_DEF(UInt64), { OBJ_TYPE_DEF(UInt64) } });
	info.Add(H_TEXT("维数"), { &ObjectArray::GetDimensionCount, OBJ_TYPE_DEF(UInt64), {} });
	info.Add(H_TEXT("添加"), { &ObjectArray::Add, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(MultiVariable) } });

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
	SET_RET_BY_TYPE(HazeVariableType(HazeValueType::UInt64), arr->m_Length);
}

void ObjectArray::GetLengthOfDimension(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	x_uint64 dimension;

	GET_PARAM_START();
	GET_OBJ(arr);
	GET_PARAM(dimension);
	SET_RET_BY_TYPE(HazeVariableType(HazeValueType::UInt64), stack->GetVM()->GetInstruction()[arr->m_PcAddress + dimension + 1].Operator[0].Extra.SignData);
}

void ObjectArray::GetDimensionCount(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_OBJ(arr);
	SET_RET_BY_TYPE(HazeVariableType(HazeValueType::UInt64), arr->m_DimensionCount);
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

		auto pair = HAZE_MALLOC(arr->m_Capacity * size, GC_ObjectType::ArrayData);
		memcpy(pair.first, arr->m_Data, arr->m_Length * size);

		arr->m_Data = pair.first;
		arr->m_DataGCIndex = pair.second;
	}

	memcpy((char*)arr->m_Data + arr->m_Length * size, value, size);
	arr->m_Length += 1;

	//SET_RET_BY_TYPE(arr->m_ValueType.BaseType, value);
}

void ObjectArray::Get(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	x_uint64 offset = 0;

	GET_PARAM_START();
	GET_OBJ(arr);
	GET_PARAM(offset);

	auto size = GetSizeByHazeType(arr->m_DimensionCount > 1 ? HazeValueType::Array : arr->m_ValueType.BaseType);

	char value[8];
	memcpy(value, (char*)arr->m_Data + offset * size, size);
	SET_RET_BY_TYPE(arr->m_ValueType, value);
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