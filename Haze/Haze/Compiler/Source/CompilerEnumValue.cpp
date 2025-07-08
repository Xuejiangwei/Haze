#include "HazePch.h"
#include "CompilerEnumValue.h"
#include "CompilerEnum.h"

CompilerEnumValue::CompilerEnumValue(CompilerEnum* owner, Share<CompilerValue> value)
	: m_OwnerEnum(owner), CompilerValue(nullptr, value->GetVariableType(), value->GetVariableScope(), value->GetVariableDesc(), 0, value)
{
}

CompilerEnumValue::CompilerEnumValue(CompilerEnum* owner, CompilerModule* compilerModule, const HazeVariableType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, Share<CompilerValue> assignValue)
	: m_OwnerEnum(owner), CompilerValue(compilerModule, defineType, scope, desc, count, assignValue)
{
}

CompilerEnumValue::~CompilerEnumValue()
{
}

x_uint32 CompilerEnumValue::GetTypeId() const
{
	return m_OwnerEnum->GetTypeId();
}
