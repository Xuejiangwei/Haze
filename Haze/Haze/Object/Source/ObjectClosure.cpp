#include "HazePch.h"
#include "ObjectClosure.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"
#include "HazeMemory.h"

#define GET_OBJ(OBJ) CHECK_GET_STACK_OBJECT(OBJ, "闭包")

ObjectClosure::ObjectClosure(x_uint32 gcIndex, const FunctionData* functionData, const FunctionData* refFunction, char* refStackESP)
	: GCObject(gcIndex), m_FunctionData(functionData)
{
	// 保留外部变量的引用, 因为能够引用的都是对象类型, 所以字节大小都是8
	auto pair = HazeMemory::GetMemory()->AllocaGCData(m_FunctionData->RefVariables.size() * sizeof(ClosureRefVariable), GC_ObjectType::ClosureData);
	m_Data = (ClosureRefVariable*)pair.first;
	m_DataGCIndex = pair.second;

	for (x_uint64 i = 0; i < m_FunctionData->RefVariables.size(); i++)
	{
		auto& refVariable = refFunction->Variables[m_FunctionData->RefVariables[i].first];
		((ClosureRefVariable*)m_Data + i)->Type = refVariable.Variable.Type;
		memcpy(&((ClosureRefVariable*)m_Data + i)->Object, refStackESP + refVariable.Offset, sizeof(ClosureRefVariable::Object));
	}
}

ObjectClosure::~ObjectClosure()
{
	if (m_FunctionData->RefVariables.size() > 0)
	{
		HazeMemory::GetMemory()->Remove(m_Data, m_FunctionData->RefVariables.size() * sizeof(ClosureRefVariable), m_DataGCIndex);
	}
}

AdvanceClassInfo* ObjectClosure::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Add(HAZE_CUSTOM_CALL_FUNCTION, {&ObjectClosure::CallFunction, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(MultiVariable) }});

	return &info;
}

void ObjectClosure::CallFunction(HAZE_OBJECT_CALL_PARAM)
{
	ObjectClosure* obj;

	GET_PARAM_START();
	GET_OBJ(obj);

	/*V_Array<HazeValue> params(obj->m_FunctionData->Variables.size());
	for (x_uint64 i = 0; i < params.size(); i++)
	{
		auto& type = obj->m_FunctionData->Variables[i].Variable.Type;
		GET_PARAM_ADDRESS(params[i].Value.Pointer, type.GetTypeSize());
		SetHazeValueByData(params[i], type.BaseType, value);
	}*/

	auto func = obj->m_FunctionData;
	stack->OnCall(func, paramByteSize);

	for (x_uint64 i = 0; i < func->RefVariables.size(); i++)
	{
		memcpy(&stack->m_StackMain[stack->m_EBP + func->Variables[func->RefVariables[i].second].Offset], &(obj->m_Data + i)->Object, sizeof(ClosureRefVariable::Object));
	}

	paramByteSize = -1;
}