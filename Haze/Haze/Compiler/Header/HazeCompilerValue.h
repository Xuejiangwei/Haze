#pragma once

class HazeCompilerModule;

class HazeCompilerValue : public std::enable_shared_from_this<HazeCompilerValue>
{
	friend class HazeCompilerClass;
public:
	//HazeCompilerValue();

	//HazeCompilerValue(HazeValue Value, HazeDataDesc Section);

	HazeCompilerValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, Share<HazeCompilerValue> assignValue = nullptr);

	virtual ~HazeCompilerValue();

	Share<HazeCompilerValue> GetShared() { return shared_from_this(); }

	virtual void StoreValueType(Share<HazeCompilerValue> srcValue);

	const HazeDefineType& GetValueType() const { return m_ValueType; }

	const HazeValue& GetValue() const { return m_Value; }

	HazeVariableScope GetVariableScope() const { return m_Scope; }

	HazeDataDesc GetVariableDesc() const { return m_Desc; }

	int GetCount() const { return m_Count; }

	bool IsGlobalVariable() const { return m_Scope == HazeVariableScope::Global; }

	bool IsLocalVariable() const { return m_Scope == HazeVariableScope::Local; }

	bool IsTempVariable() const { return m_Scope == HazeVariableScope::Temp; }

	bool IsFunctionAddress() const { return m_Desc == HazeDataDesc::FunctionAddress; }

	void SetScope(HazeVariableScope scope) { m_Scope = scope; }

	void SetDataDesc(HazeDataDesc desc) { m_Desc = desc; }

public:
	bool IsRegister() const { return IsRegisterDesc(m_Desc); }

	bool IsRegister(HazeDataDesc type) const { return IsRegisterDesc(m_Desc) && m_Desc == type; }

	bool IsConstant() const { return m_Desc == HazeDataDesc::Constant; }

	bool IsString() const { return m_Desc == HazeDataDesc::ConstantString; }

	bool IsClassMember() const { return IsClassPublicMember() || IsClassPrivateMember(); }

	bool IsClassPublicMember() const { return  m_Desc == HazeDataDesc::ClassMember_Local_Public; }

	bool IsClassPrivateMember() const { return  m_Desc == HazeDataDesc::ClassMember_Local_Private; }

	bool IsArrayElement() const { return m_Desc == HazeDataDesc::ArrayElement; }

	bool IsClassThis() const { return m_Desc == HazeDataDesc::ClassThis; }

	bool IsNullPtr() const { return m_Desc == HazeDataDesc::NullPtr; }

public:
	bool IsRefrence() const { return IsRefrenceType(m_ValueType.PrimaryType); }

	bool IsFunction() const { return IsFunctionType(m_ValueType.PrimaryType); }

	bool IsArray() const { return IsArrayType(m_ValueType.PrimaryType); }

	bool IsClass() const { return IsClassType(m_ValueType.PrimaryType); }

	bool IsAdvance() const { return IsAdvanceType(m_ValueType.PrimaryType); }

	virtual bool IsEnum() const { return false; }

public:
	virtual uint32 GetSize();

	bool TryGetVariableName(HString& outName);

protected:
	HazeDefineType m_ValueType;
	HazeValue m_Value;
	HazeCompilerModule* m_Module;
	HazeVariableScope m_Scope;
	HazeDataDesc m_Desc;

	int m_Count;			//ÃüÃû¼ÆÊý
};
