#include "HazePch.h"
#include "ObjectString.h"
#include "HazeMemory.h"
#include "MemoryHelper.h"
#include "HazeStack.h"
#include "HazeCompiler.h"
#include "HazeLibraryDefine.h"
#include "HazeStream.h"

ObjectString::ObjectString(const HChar* str, bool fixedCapacity)
	: ObjectBase(GCObjectType::String), m_Data(nullptr), m_Length(0), m_Capacity(0)
{
	if (str)
	{
		m_Length = wcslen(str);
		m_Capacity = (m_Length + 1) * sizeof(HChar);
		if (!fixedCapacity)
		{
			m_Capacity = RoundUp(m_Length);
		}

		m_Data = HazeMemory::Alloca(m_Capacity);
		memcpy(m_Data, str, m_Length);
	}
}

ObjectString::~ObjectString()
{
}

AdvanceClassInfo* ObjectString::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Functions[H_TEXT("接")] = { AdvanceFunctionType::ObjectFunction, &ObjectString::Append, HazeValueType::Void, { HazeValueType::String } };


	info.Functions[H_TEXT("格式化")] = { AdvanceFunctionType::TypeFunction, &ObjectString::Format, HazeValueType::String,
		{ HazeValueType::String, HazeValueType::MultiVariable } };
	return &info;
}

void ObjectString::Append(HazeStack* stack)
{
	ObjectString* thisStr;
	ObjectString* str;
	GET_PARAM_START();
	GET_PARAM(thisStr);
	GET_PARAM(str);

	if (thisStr->m_Length + str->m_Length <= thisStr->m_Capacity)
	{
		memcpy((HChar*)thisStr->m_Data + sizeof(HChar) * thisStr->m_Length, str, str->m_Length);
	}
}

void ObjectString::Format(HAZE_STD_CALL_PARAM)
{
	HazeStream::GetFormatString(HAZE_STD_CALL_PARAM_VAR);
}
