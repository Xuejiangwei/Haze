#include "HazePch.h"
#include "ObjectDynamicClass.h"
#include "Compiler.h"
#include "HazeStack.h"
#include "HazeLibraryDefine.h"
#include "ObjectString.h"

ObjectDynamicClass::ObjectDynamicClass(CustomMethods* methods, void* dataPtr)
	: m_Methods(methods), m_Data(dataPtr)
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
	info.Functions[HAZE_CUSTOM_GET_MEMBER] = { &ObjectDynamicClass::GetMember, HazeValueType::DynamicClassUnknow, { HazeValueType::UInt64 } };
	info.Functions[HAZE_CUSTOM_SET_MEMBER] = { &ObjectDynamicClass::SetMember, HazeValueType::Void, { HazeValueType::UInt64 } };
	info.Functions[HAZE_CUSTOM_CALL_FUNCTION] = { &ObjectDynamicClass::CallFunction, HazeValueType::DynamicClassUnknow, { HazeValueType::UInt64, HazeValueType::MultiVariable } };

	return &info;
}

void ObjectDynamicClass::GetMember(HAZE_STD_CALL_PARAM)
{
	ObjectDynamicClass* obj;
	HString* name;

	GET_PARAM_START();
	GET_PARAM(obj);
	GET_PARAM(name);

	obj->m_Methods->GetMember(stack, *name, obj->m_Data);
}

void ObjectDynamicClass::SetMember(HAZE_STD_CALL_PARAM)
{
	ObjectDynamicClass* obj;
	HString* name;

	GET_PARAM_START();
	GET_PARAM(obj);
	GET_PARAM(name);

	obj->m_Methods->SetMember(stack, *name, obj->m_Data, (x_uint8*)GET_CURRENT_ADDRESS);
	SET_RET_BY_TYPE(HazeValueType::Void, obj);
}

void ObjectDynamicClass::CallFunction(HAZE_STD_CALL_PARAM)
{
	ObjectDynamicClass* obj;
	HString* name;

	GET_PARAM_START();
	GET_PARAM(obj);
	GET_PARAM(name);

	obj->m_Methods->CallFunction(stack, *name, obj->m_Data, (x_uint8*)GET_CURRENT_ADDRESS);
}