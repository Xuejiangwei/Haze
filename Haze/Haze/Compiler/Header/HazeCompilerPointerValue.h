#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerPointerValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count);

	virtual ~HazeCompilerPointerValue() override;

	virtual void StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue) override;

	void InitPointerTo(HazeCompilerValue* PointerToValue);

	HazeCompilerValue* GetPointerValue() const { return PointerValue; }

	const HazeDefineType& GetPointerType() const { return PointerType; }

private:
	HazeDefineType PointerType;
	
	HazeCompilerValue* PointerValue;
};