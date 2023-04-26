#include "HazeCompilerArrayValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeLog.h"

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
	
	if (SrcValue->IsArray())
	{
		InitPointerTo(SrcValue);
	}
	else if (SrcValue->IsPointer())
	{
		auto SrcPointer = std::dynamic_pointer_cast<HazeCompilerPointerValue>(SrcValue);
		InitPointerTo(SrcPointer->GetPointerValue());
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("Store pointer error!\n"));
	}
}

void HazeCompilerPointerValue::InitPointerTo(HazeCompilerValue* PointerToValue)
{
	PointerValue = PointerToValue;
}

void HazeCompilerPointerValue::InitPointerTo(std::shared_ptr<HazeCompilerValue> PointerToValue)
{
	PointerValue = PointerToValue.get();
}
