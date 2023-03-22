#pragma once

#include "Haze.h"
#include "HazeCompilerValue.h"

class HazeCompilerClass;

class HazeCompilerClassValue : public HazeCompilerValue
{
public:
	HazeCompilerClassValue(HazeCompilerModule* Module, const HazeDefineData& DefineType, InstructionScopeType Scope, std::shared_ptr<HazeCompilerValue> Parent);

	virtual ~HazeCompilerClassValue() override;

	void SetUseClassMember(int Offset, HazeDefineData& Data);

private:
	std::shared_ptr<HazeCompilerClass> OwnerClass;
	
	std::pair<int, HazeDefineData> CurrUseMember;
};

