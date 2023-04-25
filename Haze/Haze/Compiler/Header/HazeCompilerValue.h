#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;

class HazeCompilerValue
{
public:
	HazeCompilerValue();

	HazeCompilerValue(HazeValue Value, HazeDataDesc Section);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count);

	virtual ~HazeCompilerValue();

	virtual void StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue);

	HazeValue& GetValue() { return Value; }

	HazeDataDesc GetScope() const { return Scope; }

	int GetCount() const { return Count; }

	bool IsRegister() const { return IsRegisterScope(Scope); }

	bool IsRegister(HazeDataDesc Type) const { return IsRegisterScope(Scope) && Scope == Type; }

	bool IsConstant() const { return Scope == HazeDataDesc::Constant; }

	bool IsGlobal() const { return Scope == HazeDataDesc::Global; }

	bool IsLocal() const { return Scope == HazeDataDesc::Local; }

	bool IsString() const { return Scope == HazeDataDesc::ConstantString; }

	bool IsClassMember() const { return IsClassPublicMember() || IsClassPrivateMember() || IsClassProtectedMember(); }

	bool IsClassPublicMember() const { return  Scope == HazeDataDesc::ClassMember_Local_Public; }

	bool IsClassPrivateMember() const { return  Scope == HazeDataDesc::ClassMember_Local_Private; }

	bool IsClassProtectedMember() const { return  Scope == HazeDataDesc::ClassMember_Local_Protected; }

	bool IsArrayElement() const { return Scope == HazeDataDesc::ArrayElement; }

	bool IsCalssThis() const { return Scope == HazeDataDesc::ClassThis; }

	bool IsPointer() const { return IsPointerBase() || IsPointerClass(); }
	
	bool IsPointerBase() const { return Value.Type == HazeValueType::PointerBase; }

	bool IsPointerClass() const { return Value.Type == HazeValueType::PointerClass; }

	bool IsArray() const { return Value.Type == HazeValueType::Array; }

	bool IsClass() const { return Value.Type == HazeValueType::Class; }

public:
	virtual uint32 GetSize();

protected:
	HazeValue Value;
	HazeCompilerModule* Module;
	HazeDataDesc Scope;

	int Count;			//ÃüÃû¼ÆÊý
};
