#pragma once
#include "HazeDefine.h"

enum class GCObjectType : uint8
{
	Base_NoGC,
	Array,
	String,
	Class,
};

class ObjectBase
{
public:
	ObjectBase(GCObjectType type) :
		m_Type(type)
	{}

	~ObjectBase() {}

	const GCObjectType GetObjectType() const { return m_Type; }

private:
	GCObjectType m_Type;

};