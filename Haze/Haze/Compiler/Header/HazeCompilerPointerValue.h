#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerPointerValue : public HazeCompilerValue
{
public:
	HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineData& DefineType, InstructionScopeType Scope, std::shared_ptr<HazeCompilerValue> Parent);

	~HazeCompilerPointerValue();

	bool IsHazeTypePointer();

	bool IsClassPointer();

private:
	HazeDefineData PointerType;
	
	std::shared_ptr<HazeCompilerValue> PointerValue;
};