#pragma once
#include "ObjectDefine.h"
#include "HazeInstruction.h"

class ObjectClass : public ObjectBase
{
public:
	ObjectClass(ClassData* classInfo) :
		ObjectBase(GCObjectType::Class), m_ClassInfo(classInfo)
	{}

	~ObjectClass() {}

private:
	ClassData* m_ClassInfo;
};
