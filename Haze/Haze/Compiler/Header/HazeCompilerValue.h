#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;

class HazeCompilerValue
{
public:
	HazeCompilerValue();

	HazeCompilerValue(HazeValue Value, InstructionScopeType Section);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, InstructionScopeType Scope);

	virtual ~HazeCompilerValue();

	void StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue);

	HazeValue& GetValue() { return Value; }

	InstructionScopeType GetScope() { return Scope; }

	bool IsRegister() { return IsRegisterScope(Scope); }

	bool IsConstant() { return Scope == InstructionScopeType::Constant; }

	bool IsGlobal() { return Scope == InstructionScopeType::Global; }

	bool IsLocal() { return Scope == InstructionScopeType::Local; }

	bool IsTemp() { return Scope == InstructionScopeType::Temp; }

	bool IsString() { return Scope == InstructionScopeType::ConstantString; }

	bool IsClassMember() { return Scope == InstructionScopeType::ClassMember_Local; }

	bool IsPointer() { return Value.Type == HazeValueType::PointerBase || Value.Type == HazeValueType::PointerClass; }

	bool IsClass() { return Value.Type == HazeValueType::Class; }

public:
	virtual unsigned int GetSize();

protected:
	HazeValue Value;
	HazeCompilerModule* Module;
	InstructionScopeType Scope;
};
