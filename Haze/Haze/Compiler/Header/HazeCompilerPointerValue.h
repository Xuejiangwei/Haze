#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerPointerValue : public HazeCompilerValue
{
public:
	HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope);

	virtual ~HazeCompilerPointerValue() override;

	void InitPointerTo(HazeCompilerValue* PointerToValue);

	HazeCompilerValue* GetPointerValue() { return PointerValue; }

	bool IsHazeTypePointer();

	bool IsClassPointer();

	const HazeDefineType& GetPointerType() { return PointerType; }

private:
	HazeDefineType PointerType;
	
	HazeCompilerValue* PointerValue;
};