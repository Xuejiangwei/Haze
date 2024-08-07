#include "HazePch.h"
#include "ObjectClass.h"
#include "HazeMemory.h"

ObjectClass::ObjectClass(ClassData* classInfo)
	: m_ClassInfo(classInfo)
{
	m_Data = HazeMemory::Alloca(classInfo->Size);
}
