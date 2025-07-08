#include "HazePch.h"
#include "ObjectDynamicClass.h"
#include "Compiler.h"
#include "HazeStack.h"
#include "HazeVM.h"
#include "HazeLibraryDefine.h"
#include "ObjectString.h"
#include "HazeMemory.h"

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
	info.Add(HAZE_CUSTOM_SET_MEMBER, { &ObjectDynamicClass::SetMember, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(UInt64) } });
	info.Add(HAZE_CUSTOM_CALL_FUNCTION, { &ObjectDynamicClass::CallFunction, OBJ_TYPE_DEF(DynamicClassUnknow), { OBJ_TYPE_DEF(String), OBJ_TYPE_DEF(MultiVariable) } });

	return &info;
}

void ObjectDynamicClass::GetMember(HAZE_OBJECT_CALL_PARAM)
{
	ObjectDynamicClass* obj;
	HString* name;

	GET_PARAM_START();
	GET_PARAM(obj);
	if (!obj)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("动态类对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	GET_PARAM(name);
	if (!name)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		auto& var2 = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 3].Operator[0];
		OBJECT_ERR_W("动态类对象获取<%s>成员错误", var.Variable.Name.c_str(), var2.Variable.Name.c_str());
		return;
	}

	if (!obj->m_Methods || !obj->m_Methods->IsValid())
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("动态类对象<%s>的获取函数表为空", var.Variable.Name.c_str());
		return;
	}

	obj->m_Methods->GetMember(stack, *name, obj->m_Data);
}

void ObjectDynamicClass::SetMember(HAZE_OBJECT_CALL_PARAM)
{
	ObjectDynamicClass* obj;
	HString* name;

	GET_PARAM_START();
	GET_PARAM(obj);
	if (!obj)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("动态类对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	GET_PARAM(name);
	if (!name)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		auto& var2 = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 3].Operator[0];
		OBJECT_ERR_W("动态类对象设置<%s>成员错误", var.Variable.Name.c_str(), var2.Variable.Name.c_str());
		return;
	}

	if (!obj->m_Methods || !obj->m_Methods->IsValid())
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("动态类对象<%s>的设置函数表为空", var.Variable.Name.c_str());
		return;
	}

	obj->m_Methods->SetMember(stack, *name, obj->m_Data, (x_uint8*)GET_CURRENT_ADDRESS);
	SET_RET_BY_TYPE(HazeValueType::Void, obj);
}

void ObjectDynamicClass::CallFunction(HAZE_OBJECT_CALL_PARAM)
{
	ObjectDynamicClass* obj;
	HString* name;

	GET_PARAM_START();
	GET_PARAM(obj);
	if (!obj)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("动态类对象<%s>为空", var.Variable.Name.c_str());
		return;
	}

	GET_PARAM(name);
	if (!name)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		auto& var2 = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 3].Operator[0];
		OBJECT_ERR_W("动态类对象调用<%s>函数错误", var.Variable.Name.c_str(), var2.Variable.Name.c_str());
		return;
	}

	if (!obj->m_Methods || !obj->m_Methods->IsValid())
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("动态类对象<%s>的调用函数表为空", var.Variable.Name.c_str());
		return;
	}

	obj->m_Methods->CallFunction(stack, *name, obj->m_Data, (x_uint8*)GET_CURRENT_ADDRESS);
}