#include "HazePch.h"
#include "ObjectString.h"
#include "HazeMemory.h"
#include "MemoryHelper.h"
#include "HazeStack.h"
#include "HazeVM.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"
#include "HazeStream.h"

#define STR_LEN_SIZE(STR) (STR->m_Length * sizeof(x_HChar))
#define GET_OBJ(OBJ) CHECK_GET_STACK_OBJECT(OBJ, "字符串")


ObjectString::ObjectString(x_uint32 gcIndex, const x_HChar* str, bool fixedCapacity)
	: GCObject(gcIndex), m_Data(nullptr), m_Length(0), m_Capacity(0), m_DataGCIndex(0)
{
	if (str)
	{
		m_Length = wcslen(str);
		m_Capacity = m_Length + 1;
		if (!fixedCapacity)
		{
			if (m_Length < 128)
			{
				m_Length = RoundUp(m_Length);
				m_Capacity = m_Length + 1;
			}
		}

		auto pair = HAZE_MALLOC(GetCapacityByteSize(), GC_ObjectType::StringData);
		m_Data = pair.first;
		m_DataGCIndex = pair.second;
		memcpy(m_Data, str, STR_LEN_SIZE(this));
	}
}

ObjectString::~ObjectString()
{
	HAZE_FREE(m_Data, GetCapacityByteSize(), m_DataGCIndex);
}

AdvanceClassInfo* ObjectString::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Add(H_TEXT("接"), { &ObjectString::Append, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(String) } });
	info.Add(H_TEXT("格式化"), { &ObjectString::Format, OBJ_TYPE_DEF(String), { OBJ_TYPE_DEF(String), OBJ_TYPE_DEF(MultiVariable) } });

	return &info;
}

bool ObjectString::IsEqual(ObjectString* obj1, ObjectString* obj2)
{
	return obj1 && obj2 && HString((x_HChar*)obj1->m_Data) == HString((x_HChar*)obj2->m_Data);
}

ObjectString* ObjectString::Create(const x_HChar* str, bool fixedCapacity)
{
	auto allocaData = HAZE_MALLOC(sizeof(ObjectString), GC_ObjectType::String);
	return new(allocaData.first) ObjectString(allocaData.second, str, fixedCapacity);
}

x_uint64 ObjectString::Hash() const
{
	// FNV-1a哈希算法实现
	static constexpr x_uint32 FNV_PRIME = 0x01000193;
	static constexpr x_uint32 FNV_OFFSET_BASIS = 0x811C9DC5;

	uint32_t hash = FNV_OFFSET_BASIS;
	for (x_uint64 i = 0; i < m_Length; ++i)
	{
		hash ^= static_cast<x_uint32>(((x_HChar*)m_Data)[i]);
		hash *= FNV_PRIME;
	}

	return hash;
}

void ObjectString::Append(HAZE_OBJECT_CALL_PARAM)
{
	ObjectString* thisStr;
	ObjectString* str;
	GET_PARAM_START();
	GET_OBJ(thisStr);

	GET_PARAM(str);

	if (thisStr->m_Length + str->m_Length < thisStr->m_Capacity)
	{
		memcpy((x_HChar*)thisStr->m_Data + thisStr->m_Length, str->m_Data, STR_LEN_SIZE(str));
		thisStr->m_Length += str->m_Length;
		thisStr->BeEnd();
	}
	else
	{
		auto oldLength = thisStr->m_Length;
		thisStr->m_Length += str->m_Length;
		thisStr->m_Capacity = thisStr->m_Length + 1;

		auto pair = HAZE_MALLOC(thisStr->GetCapacityByteSize(), GC_ObjectType::StringData);
		memcpy(pair.first, thisStr->m_Data, oldLength* sizeof(x_HChar));
		memcpy((x_HChar*)pair.first + oldLength, str->m_Data, STR_LEN_SIZE(str));
		
		thisStr->m_Data = pair.first;
		thisStr->m_DataGCIndex = pair.second;
	}
}

void ObjectString::Format(HAZE_OBJECT_CALL_PARAM)
{
	auto str = HazeStream::GetObjectFormatString(HAZE_STD_CALL_PARAM_VAR);

	ObjectString* thisStr;
	GET_PARAM_START();
	GET_OBJ(thisStr);

	if (str.length() <= thisStr->m_Capacity)
	{
		memcpy((x_HChar*)thisStr->m_Data, str.c_str(), str.length() * sizeof(x_HChar));
	}
	else
	{
		thisStr->m_Length = str.length();
		thisStr->m_Capacity = thisStr->m_Length + 1;

		auto pair = HAZE_MALLOC(thisStr->GetCapacityByteSize(), GC_ObjectType::StringData);
		memcpy(pair.first, str.c_str(), str.length() * sizeof(x_HChar));

		thisStr->m_Data = pair.first;
		thisStr->m_DataGCIndex = pair.second;
	}
}
