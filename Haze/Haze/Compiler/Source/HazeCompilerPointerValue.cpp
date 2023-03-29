#include "HazeCompilerPointerValue.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, InstructionScopeType Scope)
	: HazeCompilerValue(Module, DefineType, Scope), PointerType(DefineType)
{
}

HazeCompilerPointerValue::~HazeCompilerPointerValue()
{
}

void HazeCompilerPointerValue::InitPointerTo(HazeCompilerValue* PointerToValue)
{
	PointerValue = PointerToValue;
}

bool HazeCompilerPointerValue::IsHazeTypePointer()
{
	return Value.Type == HazeValueType::PointerBase;
}

bool HazeCompilerPointerValue::IsClassPointer()
{
	return  Value.Type == HazeValueType::PointerClass;
}