#pragma once

#include <memory>
#include "HazeHeader.h"

class HazeCompilerModule;

class HazeCompilerValue : public std::enable_shared_from_this<HazeCompilerValue>
{
	friend class HazeCompilerClass;
public:
	//HazeCompilerValue();

	//HazeCompilerValue(HazeValue Value, HazeDataDesc Section);

	HazeCompilerValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, std::shared_ptr<HazeCompilerValue> assignValue = nullptr);

	virtual ~HazeCompilerValue();

	std::shared_ptr<HazeCompilerValue> GetShared() { return shared_from_this(); }

	virtual void StoreValueType(std::shared_ptr<HazeCompilerValue> srcValue);

	virtual void StoreValue(HazeValue& srcValue);

	const HazeDefineType& GetValueType() const { return m_ValueType; }

	const HazeValue& GetValue() const { return m_Value; }

	HazeVariableScope GetVariableScope() const { return m_Scope; }

	HazeDataDesc GetVariableDesc() const { return m_Desc; }

	int GetCount() const { return m_Count; }

	bool IsGlobalVariable() const { return m_Scope == HazeVariableScope::Global; }

	bool IsLocalVariable() const { return m_Scope == HazeVariableScope::Local; }

	bool IsTempVariable() const { return m_Scope == HazeVariableScope::Temp; }

	void SetScope(HazeVariableScope scope) { m_Scope = scope; }
public:
	bool IsRegister() const { return IsRegisterDesc(m_Desc); }

	bool IsRegister(HazeDataDesc type) const { return IsRegisterDesc(m_Desc) && m_Desc == type; }

	bool IsConstant() const { return m_Desc == HazeDataDesc::Constant; }

	bool IsString() const { return m_Desc == HazeDataDesc::ConstantString; }

	bool IsClassMember() const { return IsClassPublicMember() || IsClassPrivateMember() || IsClassProtectedMember(); }

	bool IsClassPublicMember() const { return  m_Desc == HazeDataDesc::ClassMember_Local_Public; }

	bool IsClassPrivateMember() const { return  m_Desc == HazeDataDesc::ClassMember_Local_Private; }

	bool IsClassProtectedMember() const { return  m_Desc == HazeDataDesc::ClassMember_Local_Protected; }

	bool IsArrayElement() const { return m_Desc == HazeDataDesc::ArrayElement; }

	bool IsCalssThis() const { return m_Desc == HazeDataDesc::ClassThis; }

	bool IsNullPtr() const { return m_Desc == HazeDataDesc::NullPtr; }

public:
	bool IsPointer() const { return IsPointerType(m_ValueType.PrimaryType); }

	bool IsPointerBase() const { return m_ValueType.PrimaryType == HazeValueType::PointerBase; }

	bool IsPointerClass() const { return m_ValueType.PrimaryType == HazeValueType::PointerClass; }

	bool IsPointerFunction() const { return m_ValueType.PrimaryType == HazeValueType::PointerFunction; }

	bool IsPointerPointer() const { return m_ValueType.PrimaryType == HazeValueType::PointerPointer; }

	bool IsRefBase() const { return m_ValueType.PrimaryType == HazeValueType::ReferenceBase; }

	bool IsRefClass() const { return m_ValueType.PrimaryType == HazeValueType::ReferenceClass; }

	bool IsRef() const { return IsRefBase() || IsRefClass(); }

	bool IsArray() const { return IsArrayType(m_ValueType.PrimaryType); }

	bool IsClass() const { return m_ValueType.PrimaryType == HazeValueType::Class; }

public:
	virtual uint32 GetSize();

	bool TryGetVariableName(HAZE_STRING& outName);

protected:
	HazeDefineType m_ValueType;
	HazeValue m_Value;
	HazeCompilerModule* m_Module;
	HazeVariableScope m_Scope;
	HazeDataDesc m_Desc;

	int m_Count;			//ÃüÃû¼ÆÊý
};
