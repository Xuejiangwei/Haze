#include "HazeCompilerPointerValue.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count)
	: HazeCompilerValue(Module, DefineType, Scope, Count), PointerType(DefineType)
{
}

HazeCompilerPointerValue::~HazeCompilerPointerValue()
{
}

void HazeCompilerPointerValue::InitPointerTo(HazeCompilerValue* PointerToValue)
{
	PointerValue = PointerToValue;
}