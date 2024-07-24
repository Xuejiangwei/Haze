#pragma once
#include "ObjectDefine.h"

class ObjectArray : public ObjectBase
{
public:
	ObjectArray() :
		ObjectBase(GCObjectType::Array), m_Objects(nullptr)
	{}

	~ObjectArray()
	{}

private:
	uint64 m_Capacity;
	uint64 m_Length;

	ObjectBase* m_Objects;
};
