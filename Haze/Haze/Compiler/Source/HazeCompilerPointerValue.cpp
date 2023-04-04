#include "HazeCompilerPointerValue.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope)
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