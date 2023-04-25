#include "HazeCompilerPointerValue.h"

HazeCompilerPointerValue::HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count)
	: HazeCompilerValue(Module, DefineType, Scope, Count), PointerType(DefineType), PointerValue(nullptr)
{

}

HazeCompilerPointerValue::~HazeCompilerPointerValue()
{
}

void HazeCompilerPointerValue::StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue)
{
	HazeCompilerValue::StoreValue(SrcValue);
	
	auto SrcPointer = std::dynamic_pointer_cast<HazeCompilerPointerValue>(SrcValue);
	
	InitPointerTo(SrcPointer->GetPointerValue());
}

void HazeCompilerPointerValue::InitPointerTo(HazeCompilerValue* PointerToValue)
{
	PointerValue = PointerToValue;
}