#include "HazePch.h"
#include "ObjectArray.h"
#include "HazeMemory.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"
#include "ObjectHash.h"
#include "ObjectBase.h"

ObjectArray::ObjectArray(x_uint32 gcIndex, x_uint64 dimensionCount, x_uint64* lengths, x_uint64 pcAddress, HazeValueType valueType, ClassData* classInfo)
	: GCObject(gcIndex), m_Data(nullptr), m_DimensionCount(dimensionCount), m_Length(0), m_PcAddress(pcAddress), m_ValueType(valueType),
	  m_ClassInfo(classInfo)
{
	if (dimensionCount > 1)
	{
		m_ValueType = HazeValueType::Array;
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
		HazeMemory::GetMemory()->Remove(m_Data, m_Capacity * GetSizeByHazeType(m_ValueType), m_DataGCIndex);
	}
	else
	{
		HazeMemory::GetMemory()->Remove(m_Data, m_Capacity * sizeof(ObjectArray), m_DataGCIndex);
	}
}

AdvanceClassInfo* ObjectArray::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	//info.Functions[H_TEXT("生成")] = { &ObjectArray::NewObjectArray, HazeValueType::Void, { HazeValueType::MultiVariable } };
	info.Add(H_TEXT("长度"), { &ObjectArray::GetLength, HazeValueType::UInt64, {} });
	info.Add(H_TEXT("维之长度"), { &ObjectArray::GetLengthOfDimension, HazeValueType::UInt64, { HazeValueType::UInt64 } });
	info.Add(H_TEXT("维数"), { &ObjectArray::GetDimensionCount, HazeValueType::UInt64, {} });
	info.Add(H_TEXT("添加"), { &ObjectArray::Add, HazeValueType::Void, { HazeValueType::MultiVariable } });

	info.Add(HAZE_ADVANCE_GET_FUNCTION, { &ObjectArray::Get, HazeValueType::Void, { HazeValueType::UInt64 } });
	info.Add(HAZE_ADVANCE_SET_FUNCTION, { &ObjectArray::Set, HazeValueType::Void, { HazeValueType::UInt64, HazeValueType::MultiVariable } });

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
	GET_PARAM(arr);
	if (!arr)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 1].Operator[0];
		OBJECT_ERR_W("数组对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	SET_RET_BY_TYPE(HazeValueType::UInt64, arr->m_Length);
}

bool ObjectBase::IsEqual(ObjectBase* obj1, ObjectBase* obj2)
{
	return false;
}

void ObjectArray::GetLengthOfDimension(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	x_uint64 dimension;

	GET_PARAM_START();
	GET_PARAM(arr);
	if (!arr)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("数组对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	GET_PARAM(dimension);
	SET_RET_BY_TYPE(HazeValueType::UInt64, stack->GetVM()->GetInstruction()[arr->m_PcAddress + dimension + 1].Operator[0].Extra.SignData);
}

void ObjectArray::GetDimensionCount(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;

	GET_PARAM_START();
	GET_PARAM(arr);
	if (!arr)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 1].Operator[0];
		OBJECT_ERR_W("数组对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	SET_RET_BY_TYPE(HazeValueType::UInt64, arr->m_DimensionCount);
}

void ObjectArray::Add(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	char* value = nullptr;

	GET_PARAM_START();
	GET_PARAM(arr);
	if (!arr)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("数组对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	auto size = GetSizeByHazeType(arr->m_ValueType);
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

	SET_RET_BY_TYPE(arr->m_ValueType, value);
}

void ObjectArray::Get(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	x_uint64 offset = 0;

	GET_PARAM_START();
	GET_PARAM(arr);
	if (!arr)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("数组对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	GET_PARAM(offset);

	auto size = GetSizeByHazeType(arr->m_ValueType);

	char value[8];
	memcpy(value, (char*)arr->m_Data + offset * size, size);
	SET_RET_BY_TYPE(arr->m_ValueType, value);
	//HAZE_LOG_INFO("Array Get <%d>\n", offset);
}

void ObjectArray::Set(HAZE_OBJECT_CALL_PARAM)
{
	ObjectArray* arr;
	x_uint64 offset = 0;
	char* value = nullptr;

	GET_PARAM_START();
	GET_PARAM(arr);
	if (!arr)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("数组对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	GET_PARAM(offset);

	auto size = GetSizeByHazeType(arr->m_ValueType);
	GET_PARAM_ADDRESS(value, size);

	memcpy((char*)arr->m_Data + offset * size, value, size);
	//HAZE_LOG_INFO("Array Set <%d>\n", offset);
}