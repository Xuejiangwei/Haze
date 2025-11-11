#include "HazePch.h"
#include "ObjectDynamicClass.h"
#include "Compiler.h"
#include "HazeStack.h"
#include "HazeVM.h"
#include "HazeLibraryDefine.h"
#include "ObjectString.h"
#include "HazeMemory.h"

#define GET_OBJ(OBJ) CHECK_GET_STACK_OBJECT(OBJ, "动态类对象")
#define CHECK_METHOD() if (!obj->m_Methods || !obj->m_Methods->IsValid()) { OBJECT_ERR_W("动态类对象的调用函数表为空"); return; }

ObjectDynamicClass::ObjectDynamicClass(x_uint32 gcIndex, CustomMethods* methods, void* dataPtr)
	: GCObject(gcIndex), m_Methods(methods), m_Data(dataPtr)
{
	if (methods)
	{
		if (methods->IsValid())
		{
			m_Methods->Constructor(m_Data);
		}
	}
}

ObjectDynamicClass::~ObjectDynamicClass()
{
	if (m_Methods)
	{
		m_Methods->Deconstructor(m_Data);
		m_Data = nullptr;
	}
}

AdvanceClassInfo* ObjectDynamicClass::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Add(HAZE_CUSTOM_GET_MEMBER, { &ObjectDynamicClass::GetMember, OBJ_TYPE_DEF(DynamicClassUnknow), { OBJ_TYPE_DEF(UInt64) } });
	info.Add(HAZE_CUSTOM_SET_MEMBER, { &ObjectDynamicClass::SetMember, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(String), OBJ_TYPE_DEF(DynamicClassUnknow)}});
	info.Add(HAZE_CUSTOM_CALL_FUNCTION, { &ObjectDynamicClass::CallFunction, OBJ_TYPE_DEF(DynamicClassUnknow), { OBJ_TYPE_DEF(String), OBJ_TYPE_DEF(MultiVariable) } });

	return &info;
}

void ObjectDynamicClass::GetMember(HAZE_OBJECT_CALL_PARAM)
{
	ObjectDynamicClass* obj;
	ObjectString* name;

	GET_PARAM_START();
	GET_OBJ(obj);
	GET_PARAM(name);
	
	CHECK_METHOD();
	obj->m_Methods->GetMember(stack, name->GetData(), obj->m_Data);
}

void ObjectDynamicClass::SetMember(HAZE_OBJECT_CALL_PARAM)
{
	ObjectDynamicClass* obj;
	ObjectString* name;

	GET_PARAM_START_WITH_RET();
	GET_OBJ(obj);
	GET_PARAM(name);

	CHECK_METHOD();
	obj->m_Methods->SetMember(stack, name->GetData(), obj->m_Data, (x_uint8*)GET_CURRENT_ADDRESS);
	SET_RET_BY_TYPE(HazeVariableType(HazeValueType::Void), obj);
}

void ObjectDynamicClass::CallFunction(HAZE_OBJECT_CALL_PARAM)
{
	ObjectDynamicClass* obj;
	ObjectString* name;

	GET_PARAM_START();
	GET_OBJ(obj);
	GET_PARAM(name);
	
	CHECK_METHOD();
	obj->m_Methods->CallFunction(stack, name->GetData(), obj->m_Data, (x_uint8*)GET_CURRENT_ADDRESS);
}