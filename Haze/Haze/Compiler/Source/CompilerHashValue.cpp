#include "HazePch.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerHashValue.h"
#include "CompilerArrayValue.h "

CompilerHashValue::CompilerHashValue(CompilerModule* compilerModule, const HazeVariableType& defineType, HazeVariableScope scope, HazeDataDesc desc, int count)
	: CompilerValue(compilerModule, defineType, scope, desc, count)
{
	auto typeInfoMap = m_Module->GetCompiler()->GetTypeInfoMap();
	auto typeInfo = (HazeComplexTypeInfo::Hash*)typeInfoMap->GetTypeInfoById(defineType.TypeId);

	auto keyTypeInfo = typeInfoMap->GetTypeById(typeInfo->TypeId1);
	auto valueTypeInfo = typeInfoMap->GetTypeById(typeInfo->TypeId2);
	m_KeyType.BaseType = keyTypeInfo->GetBaseType();
	m_KeyType.TypeId = typeInfo->TypeId1;
	m_ValueType.BaseType = valueTypeInfo->GetBaseType();
	m_ValueType.TypeId = typeInfo->TypeId2;
}

CompilerHashValue::~CompilerHashValue()
{
}

bool CompilerHashValue::IsKeyType(Share<CompilerValue> key)
{
	return m_KeyType == key->GetVariableType();
}

bool CompilerHashValue::IsValueType(Share<CompilerValue> value)
{
	return m_ValueType == value->GetVariableType();
}