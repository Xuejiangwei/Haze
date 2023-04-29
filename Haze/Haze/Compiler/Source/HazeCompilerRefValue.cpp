#include "HazeCompilerRefValue.h"

HazeCompilerRefValue::HazeCompilerRefValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, std::shared_ptr<HazeCompilerValue>& RefValue)
	: HazeCompilerValue(Module, DefineType, Scope, Count), RefValue(RefValue)
{
}

HazeCompilerRefValue::~HazeCompilerRefValue()
{
}

void HazeCompilerRefValue::StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue)
{
	HazeCompilerValue::StoreValue(SrcValue);
}