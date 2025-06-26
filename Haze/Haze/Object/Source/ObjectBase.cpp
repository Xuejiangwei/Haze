#include "HazePch.h"
#include "ObjectBase.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"

#define GET_OBJ(OBJ) CHECK_GET_STACK_OBJECT(OBJ, "基本类型")


ObjectBase::ObjectBase(x_uint32 gcIndex, HazeValueType type)
	: GCObject(gcIndex), m_Type(type)
{
}

ObjectBase::~ObjectBase()
{
}

AdvanceClassInfo* ObjectBase::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;

	info.Add(HAZE_OBJECT_BASE_CONSTRUCTOR, { &ObjectBase::Constructor, HazeValueType::Void, { HazeValueType::MultiVariable } });
	info.Add(H_TEXT("等于"), { &ObjectBase::Equal, HazeValueType::Bool, { HazeValueType::ObjectBase } });
	info.Add(HAZE_ADVANCE_GET_FUNCTION, { &ObjectBase::Get, HazeValueType::Void, { } });
	info.Add(HAZE_ADVANCE_SET_FUNCTION, { &ObjectBase::Set, HazeValueType::Void, { HazeValueType::MultiVariable } });

	return &info;
}

bool ObjectBase::IsEqual(ObjectBase* obj1, ObjectBase* obj2)
{
	return obj1 && obj2 && obj1->m_Type == obj2->m_Type && IsEqualByType(obj1->m_Type, obj1->m_Value, obj2->m_Value);
}

void ObjectBase::Constructor(HAZE_OBJECT_CALL_PARAM)
{
	Set(HAZE_STD_CALL_PARAM_VAR);
}

void ObjectBase::Equal(HAZE_OBJECT_CALL_PARAM)
{
	ObjectBase* obj1;
	ObjectBase* obj2;

	GET_PARAM_START();
	GET_OBJ(obj1);
	GET_OBJ(obj2);

	bool isEqual = IsEqual(obj1, obj2);
	SET_RET_BY_TYPE(HazeValueType::Bool, isEqual);
}

void ObjectBase::Get(HAZE_OBJECT_CALL_PARAM)
{
	ObjectBase* obj;

	GET_PARAM_START();
	GET_OBJ(obj);
	SET_RET_BY_TYPE(obj->m_Type, obj->m_Value);
}

void ObjectBase::Set(HAZE_OBJECT_CALL_PARAM)
{
	ObjectBase* obj;
	char* data;

	GET_PARAM_START();
	GET_OBJ(obj);

	GET_PARAM_ADDRESS(data, GetSizeByHazeType(obj->m_Type));
	SetHazeValueByData(obj->m_Value, obj->m_Type, data);
}