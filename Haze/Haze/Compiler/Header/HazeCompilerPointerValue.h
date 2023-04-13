#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerPointerValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count);

	virtual ~HazeCompilerPointerValue() override;

	void InitPointerTo(HazeCompilerValue* PointerToValue);

	HazeCompilerValue* GetPointerValue() { return PointerValue; }

	const HazeDefineType& GetPointerType() { return PointerType; }

private:
	HazeDefineType PointerType;
	
	HazeCompilerValue* PointerValue;
};