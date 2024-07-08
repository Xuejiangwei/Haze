#include "HazePch.h"
#include "HazeCompilerEnumValue.h"
#include "HazeCompilerEnum.h"

HazeCompilerEnumValue::HazeCompilerEnumValue(HazeCompilerEnum* owner, Share<HazeCompilerValue>& value)
	: m_OwnerEnum(owner), HazeCompilerValue(nullptr, value->GetValueType(), value->GetVariableScope(),
		value->GetVariableDesc(), 0, value)
{
	
}

HazeCompilerEnumValue::~HazeCompilerEnumValue()
{
}
