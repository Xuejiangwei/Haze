#pragma once

#include "Haze.h"
#include "HazeCompilerValue.h"

class HazeCompilerClass;

class HazeCompilerClassValue : public HazeCompilerValue
{
public:
	friend class HazeCompilerClass;

	HazeCompilerClassValue(HazeCompilerModule* Module, const HazeDefineData& DefineType, InstructionScopeType Scope);

	virtual ~HazeCompilerClassValue() override;

	virtual unsigned int GetSize() override;

	HazeCompilerClass* GetOwnerClass() { return OwnerClass; };

	std::shared_ptr<HazeCompilerValue> GetMember(const HAZE_STRING& Name);

	void GetMemberName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

private:
	HazeCompilerClass* OwnerClass;
	
	std::vector<std::shared_ptr<HazeCompilerValue>> Vector_Data;
};

