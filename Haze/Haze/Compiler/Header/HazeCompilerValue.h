#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;

class HazeCompilerValue : public std::enable_shared_from_this<HazeCompilerValue>
{
	friend class HazeCompilerClass;
public:
	HazeCompilerValue();

	HazeCompilerValue(HazeValue Value, HazeDataDesc Section);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, HazeValue* DefaultValue = nullptr);

	virtual ~HazeCompilerValue();

	std::shared_ptr<HazeCompilerValue> GetShared() { return shared_from_this(); }

	virtual void StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue);

	const HazeDefineType& GetValueType() const { return ValueType; }

	const HazeValue& GetValue() const { return Value; }

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

	bool IsNullPtr() const { return Scope == HazeDataDesc::NullPtr; }

	bool IsPointer() const { return IsPointerType(ValueType.PrimaryType); }
	
	bool IsPointerBase() const { return ValueType.PrimaryType == HazeValueType::PointerBase; }

	bool IsPointerClass() const { return ValueType.PrimaryType == HazeValueType::PointerClass; }

	bool IsPointerFunction() const { return ValueType.PrimaryType == HazeValueType::PointerFunction; }
	
	bool IsPointerArray() const { return ValueType.PrimaryType == HazeValueType::PointerArray; }

	bool IsPointerPointer() const { return ValueType.PrimaryType == HazeValueType::PointerPointer; }

	bool IsRefBase() const { return ValueType.PrimaryType == HazeValueType::ReferenceBase; }

	bool IsRefClass() const { return ValueType.PrimaryType == HazeValueType::ReferenceClass; }

	bool IsRef() const { return IsRefBase() || IsRefClass(); }

	bool IsArray() const { return ValueType.PrimaryType == HazeValueType::Array; }

	bool IsClass() const { return ValueType.PrimaryType == HazeValueType::Class; }

public:
	virtual uint32 GetSize();

protected:
	HazeDefineType ValueType;
	HazeValue Value;
	HazeCompilerModule* Module;
	HazeDataDesc Scope;

	int Count;			//ÃüÃû¼ÆÊý
};
