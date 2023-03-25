#include "HazeCompilerPointerValue.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineData& DefineType, InstructionScopeType Scope)
	: HazeCompilerValue(Module, DefineType, Scope), PointerType(DefineType)
{
}

HazeCompilerPointerValue::~HazeCompilerPointerValue()
{
}

void HazeCompilerPointerValue::InitPointerTo(HazeCompilerValue* Value)
{
	PointerValue = Value;
}

bool HazeCompilerPointerValue::IsHazeTypePointer()
{
	return PointerType.CustomName.empty();
}

bool HazeCompilerPointerValue::IsClassPointer()
{
	return !PointerType.CustomName.empty();
}