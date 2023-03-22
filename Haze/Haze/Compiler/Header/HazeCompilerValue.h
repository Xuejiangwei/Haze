#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;

class HazeCompilerValue
{
public:
	HazeCompilerValue();

	HazeCompilerValue(HazeValue Value, InstructionScopeType Section);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineData& DefineType, InstructionScopeType Scope, std::shared_ptr<HazeCompilerValue> Parent);

	virtual ~HazeCompilerValue();

	void StoreValue(std::shared_ptr<HazeCompilerValue> Value);

	HazeValue& GetValue() { return Value; }

	InstructionScopeType GetScope() { return Scope; }

	bool IsRegister() { return IsRegisterScope(Scope); }

	bool IsConstant() { return Scope == InstructionScopeType::Constant; }

	bool IsGlobal() { return Scope == InstructionScopeType::Global; }

	bool IsLocal() { return Scope == InstructionScopeType::Local; }

	bool IsTemp() { return Scope == InstructionScopeType::Temp; }

	bool IsString() { return Scope == InstructionScopeType::String; }

	bool IsClass() { return Scope == InstructionScopeType::Class; }

	bool HasParent() { return Parent != nullptr; }

	const std::shared_ptr<HazeCompilerValue>& GetParent() const { return Parent; }

protected:
	std::shared_ptr<HazeCompilerValue> Parent;

	HazeValue Value;
	HazeCompilerModule* Module;
	InstructionScopeType Scope;
};
