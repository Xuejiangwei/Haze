#include "HazePch.h"
#include "ObjectString.h"
#include "HazeMemory.h"
#include "MemoryHelper.h"

ObjectString::ObjectString(const HChar* str, bool fixedCapacity)
	: ObjectBase(GCObjectType::String), m_Data(nullptr), m_Length(0), m_Capacity(0)
{
	if (str)
	{
		m_Length = wcslen(str);
		m_Capacity = m_Length + 1;
		if (!fixedCapacity)
		{
			m_Capacity = RoundUp(m_Length);
		}

		m_Data = HazeMemory::Alloca(m_Capacity);
		memcpy(m_Data, str, m_Length + 1);
	}
}

ObjectString::~ObjectString()
{
	if (m_Data)
	{
		free(m_Data);
	}
}

void ObjectString::Append(const HChar* str)
{
	auto len = wcslen(str) + 1;
	if (m_Length + len <= m_Capacity)
	{
		memcpy((HChar*)m_Data + sizeof(HChar) * m_Length, str, len);
	}
}
