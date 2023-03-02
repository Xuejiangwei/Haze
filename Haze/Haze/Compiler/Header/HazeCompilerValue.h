#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;



class HazeCompilerValue
{
public:
	HazeCompilerValue();

	//HazeCompilerValue(HazeCompilerModule* Module);

	//HazeCompilerValue(HazeCompilerModule* Module, HazeValueType Type);

	HazeCompilerValue(HazeValue Value, InstructionScopeType Section);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineData& DefineType, InstructionScopeType Scope);

	~HazeCompilerValue();

	void StoreValue(std::shared_ptr<HazeCompilerValue> Value);

	HazeValue& GetValue() { return Value; }

	bool IsRegister() { return Scope == InstructionScopeType::Register; }

	bool IsConstant() { return Scope == InstructionScopeType::Constant; }

	bool IsGlobal() { return Scope == InstructionScopeType::Global; }

	bool IsLocal() { return Scope == InstructionScopeType::Local; }

	bool IsTemp() { return Scope == InstructionScopeType::Temp; }

private:
	HazeValue Value;
	HazeCompilerModule* Module;
	InstructionScopeType Scope;
};
