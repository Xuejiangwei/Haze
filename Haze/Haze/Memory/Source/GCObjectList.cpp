#include "HazePch.h"
#include "GCObjectList.h"

GCObjectList::GCObjectList(x_uint64 initLength)
{
	m_StateList.reserve(initLength);
	m_ObjectList.reserve(initLength);
}

GCObjectList::~GCObjectList()
{
	m_StateList.clear();
	m_StateList.shrink_to_fit();
	
	m_ObjectList.clear();
	m_ObjectList.shrink_to_fit();

	m_ReuseList.clear();
	m_ReuseList.shrink_to_fit();
}

x_uint32 GCObjectList::Add(void* object, GC_ObjectType type)
{
	x_uint32 index = 0;
	if (m_ReuseList.size() > 0)
	{
		index = m_ReuseList.back();
		m_ReuseList.resize(m_ReuseList.size() - 1);
		m_ObjectList[index] = object;
		
		auto& state = m_StateList[index];
		state.State = GC_State::Black;
		state.Type = type;
		state.IsUse = true;
	}
	else if (m_ObjectList.size() <= std::numeric_limits<x_uint32>::max())
	{
		index = (x_uint32)m_ObjectList.size();
		m_ObjectList.push_back(object);

		ObjectCacheState state(GC_State::Black, type, true);
		m_StateList.push_back(state);
	}
	else
	{
		HAZE_LOG_ERR_W("GC对象列表添加错误, 超过32位无符号整数的最大值\n");
	}

	return index;
}

void GCObjectList::Remove(x_uint32 index)
{
	m_ObjectList[index] = nullptr;
	m_StateList[index].IsUse = false;

	m_ReuseList.push_back(index);
}

void GCObjectList::MarkAllWhite()
{
	for (size_t i = 0; i < m_StateList.size(); i++)
	{
		m_StateList[i].State = GC_State::White;
	}
}

void GCObjectList::MarkObjectBlack(x_uint32 index)
{
	m_StateList[index].State = GC_State::Black;
}
