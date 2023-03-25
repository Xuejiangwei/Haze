#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerPointerValue : public HazeCompilerValue
{
public:
	HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineData& DefineType, InstructionScopeType Scope);

	virtual ~HazeCompilerPointerValue() override;

	void InitPointerTo(HazeCompilerValue* Value);

	HazeCompilerValue* GetPointerValue() { return PointerValue; }

	bool IsHazeTypePointer();

	bool IsClassPointer();

	const HazeDefineData& GetPointerType() { return PointerType; }

private:
	HazeDefineData PointerType;
	
	HazeCompilerValue* PointerValue;
};