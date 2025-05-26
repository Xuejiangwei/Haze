#include "HazePch.h"
#include "ObjectString.h"
#include "HazeMemory.h"
#include "MemoryHelper.h"
#include "HazeStack.h"
#include "HazeVM.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"
#include "HazeStream.h"

ObjectString::ObjectString(const x_HChar* str, bool fixedCapacity)
	: m_Data(nullptr), m_Length(0), m_Capacity(0)
{
	if (str)
	{
		m_Length = wcslen(str);
		m_Capacity = (m_Length + 1) * sizeof(x_HChar);
		if (!fixedCapacity)
		{
			m_Capacity = RoundUp(m_Length);
		}

		m_Data = HazeMemory::Alloca(m_Capacity);
		memcpy(m_Data, str, m_Capacity);
	}
}

ObjectString::~ObjectString()
{
}

AdvanceClassInfo* ObjectString::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Add(H_TEXT("接"), { &ObjectString::Append, HazeValueType::Void, { HazeValueType::String } });
	info.Add(H_TEXT("格式化"), { &ObjectString::Format, HazeValueType::String, { HazeValueType::String, HazeValueType::MultiVariable } });

	return &info;
}

void ObjectString::Append(HAZE_STD_CALL_PARAM)
{
	ObjectString* thisStr;
	ObjectString* str;
	GET_PARAM_START();
	GET_PARAM(thisStr);
	if (!thisStr)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("字符串对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	GET_PARAM(str);

	if (thisStr->m_Length + str->m_Length <= thisStr->m_Capacity)
	{
		memcpy((x_HChar*)thisStr->m_Data + thisStr->m_Length, str, str->m_Length);
	}
	else
	{
		auto oldLength = thisStr->m_Length;
		thisStr->m_Length += str->m_Length;
		thisStr->m_Capacity = (thisStr->m_Length + 1) * sizeof(x_HChar);
		auto dataAddress = HazeMemory::Alloca(thisStr->m_Capacity);

		memcpy(dataAddress, thisStr->m_Data, oldLength* sizeof(x_HChar));
		memcpy((x_HChar*)dataAddress + oldLength, str->m_Data, str->m_Length * sizeof(x_HChar));
		
		thisStr->m_Data = dataAddress;
	}
}

void ObjectString::Format(HAZE_STD_CALL_PARAM)
{
	auto str = HazeStream::GetObjectFormatString(HAZE_STD_CALL_PARAM_VAR);

	ObjectString* thisStr;
	GET_PARAM_START();
	GET_PARAM(thisStr);
	if (!thisStr)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 1].Operator[0];
		OBJECT_ERR_W("字符串对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	if (str.length() <= thisStr->m_Capacity)
	{
		memcpy((x_HChar*)thisStr->m_Data, str.c_str(), str.length());
	}
	else
	{
		thisStr->m_Length = str.length();
		thisStr->m_Capacity = (thisStr->m_Length + 1) * sizeof(x_HChar);
		thisStr->m_Data = HazeMemory::Alloca(thisStr->m_Capacity);

		memcpy(thisStr->m_Data, str.c_str(), thisStr->m_Capacity);
	}
}
