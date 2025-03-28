#include "HazePch.h"
#include "ObjectClass.h"
#include "HazeMemory.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "Compiler.h"
#include "HazeLibraryDefine.h"

ObjectClass::ObjectClass(ClassData* classInfo)
	: m_ClassInfo(classInfo)
{
	m_Data = HazeMemory::Alloca(classInfo->Size);
	//HAZE_LOG_INFO(H_TEXT("<%s><%p> <%p> Constructor\n"), m_ClassInfo->Name.c_str(), this, m_Data);
}

AdvanceClassInfo* ObjectClass::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	//info.Functions[H_TEXT("Éú³É")] = { &ObjectArray::NewObjectArray, HazeValueType::Void, { HazeValueType::MultiVariable } };
	info.Functions[HAZE_ADVANCE_GET_FUNCTION] = { &ObjectClass::GetOffset, HazeValueType::Void, { HazeValueType::UInt64 } };
	info.Functions[HAZE_ADVANCE_SET_FUNCTION] = { &ObjectClass::SetOffset, HazeValueType::Void, { HazeValueType::UInt64, HazeValueType::MultiVariable } };

	return &info;
}

void ObjectClass::GetOffset(HAZE_STD_CALL_PARAM)
{
	ObjectClass* classObj;
	x_uint64 index = 0;

	GET_PARAM_START();
	GET_PARAM(classObj);
	GET_PARAM(index);
	
	auto& memberInfo = classObj->m_ClassInfo->Members[index];

	char value[8];
	memcpy(value, (char*)classObj->m_Data + memberInfo.Offset, GetSizeByHazeType(memberInfo.Variable.Type.PrimaryType));
	SET_RET_BY_TYPE(memberInfo.Variable.Type.PrimaryType, value);

	//HAZE_LOG_INFO(H_TEXT("Class Get <%d> <%s> <%s>\n"), index, memberInfo.Variable.Name.c_str(), classObj->m_ClassInfo->Name.c_str());
	//HAZE_LOG_INFO(H_TEXT("<%s><%p> <%p><%p> Get: <%s>\n"), classObj->m_ClassInfo->Name.c_str(), classObj, classObj->m_Data, (char*)classObj->m_Data + memberInfo.Offset, value);
}

void ObjectClass::SetOffset(HAZE_STD_CALL_PARAM)
{
	ObjectClass* classObj;
	x_uint64 index = 0;
	char* value = nullptr;

	GET_PARAM_START();
	GET_PARAM(classObj);
	GET_PARAM(index);
	
	auto& memberInfo = classObj->m_ClassInfo->Members[index];
	auto size = GetSizeByHazeType(memberInfo.Variable.Type.PrimaryType);
	GET_PARAM_ADDRESS(value, size);

	memcpy((char*)classObj->m_Data + memberInfo.Offset, value, size);

	//HAZE_LOG_INFO(H_TEXT("Class Set <%d> <%s>\n"), index, memberInfo.Variable.Name.c_str());
	//HAZE_LOG_INFO(H_TEXT("<%s><%p> <%p> Set: <%s>\n"), classObj->m_ClassInfo->Name.c_str(), classObj, classObj->m_Data, (char*)classObj->m_Data + memberInfo.Offset, value);
}
