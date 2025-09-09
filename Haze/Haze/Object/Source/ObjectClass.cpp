#include "HazePch.h"
#include "ObjectClass.h"
#include "ObjectString.h"
#include "HazeMemory.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"
#include "MemoryHelper.h"

#define GET_OBJ(OBJ) CHECK_GET_STACK_OBJECT(OBJ, "类对象")

ObjectClass::ObjectClass(x_uint32 gcIndex, HazeVM* vm, x_uint32 typeId)
	: GCObject(gcIndex)
{
	m_ClassInfo = vm->FindClass(typeId);
	auto pair = HAZE_MALLOC(m_ClassInfo->Size, GC_ObjectType::ClassData);
	m_Data = pair.first;
	m_DataGCIndex = pair.second;
	//HAZE_LOG_INFO(H_TEXT("<%s><%p> <%p> Constructor\n"), m_ClassInfo->Name.c_str(), this, m_Data);
}

ObjectClass::~ObjectClass()
{
	HAZE_FREE(m_Data, m_ClassInfo->Size, m_DataGCIndex);
}

AdvanceClassInfo* ObjectClass::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	//info.Functions[H_TEXT("生成")] = { &ObjectArray::NewObjectArray, OBJ_TYPE_DEF(::Void, { OBJ_TYPE_DEF(::MultiVariable } };
	info.Add(HAZE_ADVANCE_GET_FUNCTION, { &ObjectClass::GetOffset, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(UInt64) } });
	info.Add(HAZE_ADVANCE_SET_FUNCTION, { &ObjectClass::SetOffset, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(UInt64), OBJ_TYPE_DEF(MultiVariable) } });
	info.Add(H_TEXT("类名"), { &ObjectClass::GetClassName, OBJ_TYPE_DEF(String), { } });

	return &info;
}

bool ObjectClass::IsEqual(ObjectClass* obj1, ObjectClass* obj2)
{
	if (obj1 == obj2)
	{
		return true;
	}

	return false;
}

ObjectClass* ObjectClass::Create(HazeVM* vm, ClassData* classData)
{
	auto allocaData = HAZE_MALLOC(sizeof(ObjectClass), GC_ObjectType::Class);
	new(allocaData.first) ObjectClass(allocaData.second, vm, classData->TypeId);

	auto constructor = vm->GetFunctionDataByName(GetHazeClassFunctionName(classData->Name, classData->Name));
	if (constructor && constructor->Params.size() == 1)
	{
		vm->CallFunction(constructor, allocaData.first);
	}

	return (ObjectClass*)allocaData.first;
}

const char* ObjectClass::GetMember(const x_HChar* memberName)
{
	STDString name = memberName;
	auto& members = m_ClassInfo->Members;
	for (size_t i = 0; i < members.size(); i++)
	{
		if (members[i].Variable.Name == name)
		{
			return (char*)m_Data + members[i].Offset;
		}
	}

	return nullptr;
}

void ObjectClass::SetMember(const x_HChar* memberName, void* value)
{
	STDString name = memberName;
	auto& members = m_ClassInfo->Members;
	for (size_t i = 0; i < members.size(); i++)
	{
		if (members[i].Variable.Name == name)
		{
			auto size = GetSizeByHazeType(members[i].Variable.Type.BaseType);
			memcpy((char*)m_Data + members[i].Offset, value, size);
			break;
		}
	}
}

void ObjectClass::GetOffset(HAZE_OBJECT_CALL_PARAM)
{
	ObjectClass* classObj;
	x_uint64 index = 0;

	GET_PARAM_START();
	GET_OBJ(classObj);

	GET_PARAM(index);
	
	auto& memberInfo = classObj->m_ClassInfo->Members[index];

	char value[8];
	memcpy(value, (char*)classObj->m_Data + memberInfo.Offset, GetSizeByHazeType(memberInfo.Variable.Type.BaseType));
	SET_RET_BY_TYPE(memberInfo.Variable.Type, value);

	//HAZE_LOG_INFO(H_TEXT("Class Get <%d> <%s> <%s>\n"), index, memberInfo.Variable.Name.c_str(), classObj->m_ClassInfo->Name.c_str());
	//HAZE_LOG_INFO(H_TEXT("<%s><%p> <%p><%p> Get: <%s>\n"), classObj->m_ClassInfo->Name.c_str(), classObj, classObj->m_Data, (char*)classObj->m_Data + memberInfo.Offset, value);
}

void ObjectClass::SetOffset(HAZE_OBJECT_CALL_PARAM)
{
	ObjectClass* classObj;
	x_uint64 index = 0;
	char* value = nullptr;

	GET_PARAM_START();
	GET_OBJ(classObj);

	GET_PARAM(index);
	
	auto& memberInfo = classObj->m_ClassInfo->Members[index];
	auto size = GetSizeByHazeType(memberInfo.Variable.Type.BaseType);
	GET_PARAM_ADDRESS(value, size);

	memcpy((char*)classObj->m_Data + memberInfo.Offset, value, size);

	//HAZE_LOG_INFO(H_TEXT("Class Set <%d> <%s>\n"), index, memberInfo.Variable.Name.c_str());
	//HAZE_LOG_INFO(H_TEXT("<%s><%p> <%p> Set: <%s>\n"), classObj->m_ClassInfo->Name.c_str(), classObj, classObj->m_Data, (char*)classObj->m_Data + memberInfo.Offset, value);
}

void ObjectClass::GetClassName(HAZE_OBJECT_CALL_PARAM)
{
	ObjectClass* classObj;

	GET_PARAM_START();
	GET_OBJ(classObj);

	auto name = ObjectString::Create(classObj->m_ClassInfo->Name.c_str(), true);
	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::String), name);
}
