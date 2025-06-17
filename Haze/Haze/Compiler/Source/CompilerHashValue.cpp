#include "HazePch.h"
#include "CompilerModule.h"
#include "CompilerHashValue.h"
#include "CompilerArrayValue.h "

CompilerHashValue::CompilerHashValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope, HazeDataDesc desc, int count, TemplateDefineTypes* params)
	: CompilerValue(compilerModule, defineType, scope, desc, count)
{
	if (params && params->Types.size() == 2)
	{
		m_KeyType = params->Types[0];
		m_ValueType = params->Types[1];
	}
	else
	{
		COMPILER_ERR_W("创建哈希变量错误, 哈希类型错误");
	}
}

CompilerHashValue::~CompilerHashValue()
{
}

bool CompilerHashValue::IsKeyType(Share<CompilerValue> key)
{
	return m_KeyType.Type->BaseType == key->GetValueType();
}

bool CompilerHashValue::IsValueType(Share<CompilerValue> value)
{
	if (value->IsArray())
	{
		return m_ValueType.Type->BaseType == value->GetValueType() && DynamicCast<CompilerArrayValue>(value)->GetArrayDimension() == m_ValueType.Type->ArrayDimension;
	}

	return m_ValueType.Type->BaseType == value->GetValueType();
}

bool CompilerHashValue::KeyCanCVT(Share<CompilerValue> key)
{
	return CanCVT(m_KeyType.Type->BaseType.PrimaryType, key->GetValueType().PrimaryType);
}
