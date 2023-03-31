#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;

class HazeCompilerValue
{
public:
	HazeCompilerValue();

	HazeCompilerValue(HazeValue Value, HazeDataDesc Section);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope);

	virtual ~HazeCompilerValue();

	void StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue);

	HazeValue& GetValue() { return Value; }

	HazeDataDesc GetScope() { return Scope; }

	bool IsRegister() { return IsRegisterScope(Scope); }

	bool IsConstant() { return Scope == HazeDataDesc::Constant; }

	bool IsGlobal() { return Scope == HazeDataDesc::Global; }

	bool IsLocal() { return Scope == HazeDataDesc::Local; }

	bool IsTemp() { return Scope == HazeDataDesc::Temp; }

	bool IsString() { return Scope == HazeDataDesc::ConstantString; }

	bool IsClassMember() { return Scope == HazeDataDesc::ClassMember_Local_Public; }

	bool IsPointer() { return Value.Type == HazeValueType::PointerBase || Value.Type == HazeValueType::PointerClass; }

	bool IsClass() { return Value.Type == HazeValueType::Class; }

public:
	virtual unsigned int GetSize();

protected:
	HazeValue Value;
	HazeCompilerModule* Module;
	HazeDataDesc Scope;
};
