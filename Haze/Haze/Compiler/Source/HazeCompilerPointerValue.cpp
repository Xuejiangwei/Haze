#include "HazeCompilerPointerValue.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineData& DefineType, InstructionScopeType Scope, std::shared_ptr<HazeCompilerValue> Parent)
	: HazeCompilerValue(Module, DefineType, Scope, Parent), PointerType(DefineType)
{
}

HazeCompilerPointerValue::~HazeCompilerPointerValue()
{
}

bool HazeCompilerPointerValue::IsHazeTypePointer()
{
	return PointerType.CustomName.empty();
}

bool HazeCompilerPointerValue::IsClassPointer()
{
	return !PointerType.CustomName.empty();
}