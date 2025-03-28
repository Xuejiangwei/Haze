#include "HazePch.h"
#include "CompilerEnumValue.h"
#include "CompilerEnum.h"

CompilerEnumValue::CompilerEnumValue(CompilerEnum* owner, Share<CompilerValue> value)
	: m_OwnerEnum(owner), CompilerValue(nullptr, value->GetValueType(), value->GetVariableScope(),
		value->GetVariableDesc(), 0, value)
{
	m_ValueType.PrimaryType = HazeValueType::Enum;
	m_ValueType.SecondaryType = owner->GetParentType();
	m_ValueType.CustomName = &owner->GetName();
}

CompilerEnumValue::CompilerEnumValue(CompilerEnum* owner, CompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, Share<CompilerValue> assignValue)
	: m_OwnerEnum(owner), CompilerValue(compilerModule, defineType, scope, desc, count, assignValue)
{
	m_ValueType.SecondaryType = owner->GetParentType();
}

CompilerEnumValue::~CompilerEnumValue()
{
}

HazeValueType CompilerEnumValue::GetBaseType() const
{
	return m_OwnerEnum->GetParentType();
}

x_uint32 CompilerEnumValue::GetSize()
{
	return GetSizeByHazeType(m_ValueType.SecondaryType);
}
