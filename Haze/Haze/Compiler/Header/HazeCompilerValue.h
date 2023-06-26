#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;

class HazeCompilerValue : public std::enable_shared_from_this<HazeCompilerValue>
{
	friend class HazeCompilerClass;
public:
	//HazeCompilerValue();

	//HazeCompilerValue(HazeValue Value, HazeDataDesc Section);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, HazeValue* DefaultValue = nullptr);

	virtual ~HazeCompilerValue();

	std::shared_ptr<HazeCompilerValue> GetShared() { return shared_from_this(); }

	virtual void StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue);

	const HazeDefineType& GetValueType() const { return ValueType; }

	const HazeValue& GetValue() const { return Value; }

	HazeVariableScope GetVariableScope() const { return Scope; }
	
	HazeDataDesc GetVariableDesc() const { return Desc; }

	int GetCount() const { return Count; }

	bool IsGlobalVariable() const { return Scope == HazeVariableScope::Global; }

	bool IsLocalVariable() const { return Scope == HazeVariableScope::Local; }

public:
	bool IsRegister() const { return IsRegisterScope(Desc); }

	bool IsRegister(HazeDataDesc Type) const { return IsRegisterScope(Desc) && Desc == Type; }

	bool IsConstant() const { return Desc == HazeDataDesc::Constant; }

	bool IsString() const { return Desc == HazeDataDesc::ConstantString; }

	bool IsClassMember() const { return IsClassPublicMember() || IsClassPrivateMember() || IsClassProtectedMember(); }

	bool IsClassPublicMember() const { return  Desc == HazeDataDesc::ClassMember_Local_Public; }

	bool IsClassPrivateMember() const { return  Desc == HazeDataDesc::ClassMember_Local_Private; }

	bool IsClassProtectedMember() const { return  Desc == HazeDataDesc::ClassMember_Local_Protected; }

	bool IsArrayElement() const { return Desc == HazeDataDesc::ArrayElement; }

	bool IsCalssThis() const { return Desc == HazeDataDesc::ClassThis; }

	bool IsNullPtr() const { return Desc == HazeDataDesc::NullPtr; }

public:
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
	HazeVariableScope Scope;
	HazeDataDesc Desc;

	int Count;			//��������
};
