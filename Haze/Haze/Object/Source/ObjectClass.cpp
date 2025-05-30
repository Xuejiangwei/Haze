#include "HazePch.h"
#include "ObjectClass.h"
#include "HazeMemory.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"
#include "MemoryHelper.h"

ObjectClass::ObjectClass(x_uint32 gcIndex, ClassData* classInfo)
	: GCObject(gcIndex), m_ClassInfo(classInfo)
{
	auto pair = HazeMemory::AllocaGCData(classInfo->Size, GC_ObjectType::ClassData);
	m_Data = pair.first;
	m_DataGCIndex = pair.second;
	//HAZE_LOG_INFO(H_TEXT("<%s><%p> <%p> Constructor\n"), m_ClassInfo->Name.c_str(), this, m_Data);
}

ObjectClass::~ObjectClass()
{
	HazeMemory::GetMemory()->Remove(m_Data, m_ClassInfo->Size, m_DataGCIndex);
}

AdvanceClassInfo* ObjectClass::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	//info.Functions[H_TEXT("生成")] = { &ObjectArray::NewObjectArray, HazeValueType::Void, { HazeValueType::MultiVariable } };
	info.Add(HAZE_ADVANCE_GET_FUNCTION, { &ObjectClass::GetOffset, HazeValueType::Void, { HazeValueType::UInt64 } });
	info.Add(HAZE_ADVANCE_SET_FUNCTION, { &ObjectClass::SetOffset, HazeValueType::Void, { HazeValueType::UInt64, HazeValueType::MultiVariable } });

	return &info;
}

const char* ObjectClass::GetMember(const x_HChar* memberName)
{
	HString name = memberName;
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
	HString name = memberName;
	auto& members = m_ClassInfo->Members;
	for (size_t i = 0; i < members.size(); i++)
	{
		if (members[i].Variable.Name == name)
		{
			auto size = GetSizeByHazeType(members[i].Variable.Type.PrimaryType);
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
	GET_PARAM(classObj);
	if (!classObj)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("类<%s>对象<%s>为空", var.Variable.Type.CustomName->c_str(), var.Variable.Name.c_str());
		return;
	}

	GET_PARAM(index);
	
	auto& memberInfo = classObj->m_ClassInfo->Members[index];

	char value[8];
	memcpy(value, (char*)classObj->m_Data + memberInfo.Offset, GetSizeByHazeType(memberInfo.Variable.Type.PrimaryType));
	SET_RET_BY_TYPE(memberInfo.Variable.Type.PrimaryType, value);

	//HAZE_LOG_INFO(H_TEXT("Class Get <%d> <%s> <%s>\n"), index, memberInfo.Variable.Name.c_str(), classObj->m_ClassInfo->Name.c_str());
	//HAZE_LOG_INFO(H_TEXT("<%s><%p> <%p><%p> Get: <%s>\n"), classObj->m_ClassInfo->Name.c_str(), classObj, classObj->m_Data, (char*)classObj->m_Data + memberInfo.Offset, value);
}

void ObjectClass::SetOffset(HAZE_OBJECT_CALL_PARAM)
{
	ObjectClass* classObj;
	x_uint64 index = 0;
	char* value = nullptr;

	GET_PARAM_START();
	GET_PARAM(classObj);
	if (!classObj)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("类<%s>对象<%s>为空", var.Variable.Type.CustomName->c_str(), var.Variable.Name.c_str());
		return;
	}


	GET_PARAM(index);
	
	auto& memberInfo = classObj->m_ClassInfo->Members[index];
	auto size = GetSizeByHazeType(memberInfo.Variable.Type.PrimaryType);
	GET_PARAM_ADDRESS(value, size);

	memcpy((char*)classObj->m_Data + memberInfo.Offset, value, size);

	//HAZE_LOG_INFO(H_TEXT("Class Set <%d> <%s>\n"), index, memberInfo.Variable.Name.c_str());
	//HAZE_LOG_INFO(H_TEXT("<%s><%p> <%p> Set: <%s>\n"), classObj->m_ClassInfo->Name.c_str(), classObj, classObj->m_Data, (char*)classObj->m_Data + memberInfo.Offset, value);
}
