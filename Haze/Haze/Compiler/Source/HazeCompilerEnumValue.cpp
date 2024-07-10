#include "HazePch.h"
#include "HazeCompilerEnumValue.h"
#include "HazeCompilerEnum.h"

HazeCompilerEnumValue::HazeCompilerEnumValue(HazeCompilerEnum* owner, Share<HazeCompilerValue> value)
	: m_OwnerEnum(owner), HazeCompilerValue(nullptr, value->GetValueType(), value->GetVariableScope(),
		value->GetVariableDesc(), 0, value)
{
	m_ValueType.PrimaryType = HazeValueType::Enum;
	m_ValueType.SecondaryType = owner->GetParentType();
	m_ValueType.CustomName = owner->GetName();
}

HazeCompilerEnumValue::HazeCompilerEnumValue(HazeCompilerEnum* owner, HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, Share<HazeCompilerValue> assignValue)
	: m_OwnerEnum(owner), HazeCompilerValue(compilerModule, defineType, scope, desc, count, assignValue)
{
	m_ValueType.SecondaryType = owner->GetParentType();
}

HazeCompilerEnumValue::~HazeCompilerEnumValue()
{
}

HazeValueType HazeCompilerEnumValue::GetBaseType() const
{
	return m_OwnerEnum->GetParentType();
}

uint32 HazeCompilerEnumValue::GetSize()
{
	return GetSizeByHazeType(m_ValueType.SecondaryType);
}
