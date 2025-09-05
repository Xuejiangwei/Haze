#pragma once

class CompilerModule;

class CompilerValue : public SharedFromThis<CompilerValue>
{
	friend class CompilerClass;
	friend class CompilerValueTypeChanger;
public:
	//HazeCompilerValue();

	//HazeCompilerValue(HazeValue Value, HazeDataDesc Section);

	CompilerValue(CompilerModule* compilerModule, const HazeVariableType& defineType, /*HazeVariableScope scope,*/
		HazeDataDesc desc, int count, Share<CompilerValue> assignValue = nullptr);

	virtual ~CompilerValue();

	CompilerModule* GetModule() { return m_Module; }

	Share<CompilerValue> GetShared() { return shared_from_this(); }

	virtual HazeValueType GetBaseType() const { return m_Type.BaseType; }
	x_uint32 GetTypeId() const { return m_Type.TypeId; }
	HazeVariableType GetVariableType() const { return m_Type; }

	const HazeValue& GetValue() const { return m_Value; }

	//HazeVariableScope GetVariableScope() const { return m_Scope; }

	HazeDataDesc GetVariableDesc() const { return m_Desc; }

	int GetCount() const { return m_Count; }

	//bool IsGlobalVariable() const { return m_Scope == HazeVariableScope::Global; }
	bool IsGlobalVariable() const { return IsGlobalDesc(m_Desc); }

	//bool IsLocalVariable() const { return m_Scope == HazeVariableScope::Local; }
	bool IsLocalVariable() const { return IsLocalDesc(m_Desc); }

	bool IsTempVariable() const { return m_Desc == HazeDataDesc::RegisterTemp; }

	bool IsFunctionAddress() const { return m_Desc == HazeDataDesc::Function_Normal; }

	//void SetScope(HazeVariableScope scope) { m_Scope = scope; }

	void SetDataDesc(HazeDataDesc desc) { m_Desc = desc; }

	const STDString* GetPointerFunctionName() const;
	void SetPointerFunctionName(const STDString* name);

public:
	bool IsRegister() const { return IsRegisterDesc(m_Desc); }

	bool IsRegister(HazeDataDesc type) const { return IsRegisterDesc(m_Desc) && m_Desc == type; }

	bool IsConstant() const { return IsConstDesc(m_Desc); }

	bool IsString() const { return IsConstStringDesc(m_Desc); }

	bool IsElement() const { return m_Desc == HazeDataDesc::Element; }

	bool IsClassMember() const { return IsClassPublicMember() || IsClassPrivateMember(); }

	bool IsClassPublicMember() const { return  m_Desc == HazeDataDesc::ClassMember_Local_Public; }

	bool IsClassPrivateMember() const { return  m_Desc == HazeDataDesc::ClassMember_Local_Private; }

	bool IsClassThis() const { return m_Desc == HazeDataDesc::ClassThis; }

	bool IsNullPtr() const { return m_Desc == HazeDataDesc::NullPtr; }

public:
	bool IsRefrence() const { return IsRefrenceType(m_Type.BaseType); }

	bool IsFunction() const { return IsFunctionType(m_Type.BaseType); }

	bool IsArray() const { return IsArrayType(m_Type.BaseType); }

	bool IsDynamicClass() const { return IsDynamicClassType(m_Type.BaseType); }

	bool IsDynamicClassUnknow() const { return IsDynamicClassUnknowType(m_Type.BaseType); }

	bool IsClass() const { return IsClassType(m_Type.BaseType); }

	bool IsAdvance() const { return IsAdvanceType(m_Type.BaseType); }

	bool IsPureString() const { return IsPureStringType(m_Type.BaseType); }

	bool IsObjectBase() const { return IsObjectBaseType(m_Type.BaseType); }

	bool IsHash() const { return IsHashType(m_Type.BaseType); }
	
	bool IsClosure() const { return IsClosureType(m_Type.BaseType); }

	virtual bool IsEnum() const { return false; }

public:
	virtual x_uint32 GetSize() { return GetSizeByHazeType(m_Type.BaseType);}

	bool TryGetVariableName(HStringView& outName);
	bool TryGetVariableId(InstructionOpId& outId);

protected:
	HazeVariableType m_Type;
	HazeValue m_Value;
	CompilerModule* m_Module;
	//HazeVariableScope m_Scope;
	HazeDataDesc m_Desc;

	int m_Count;			//命名计数
};


class CompilerValueTypeChanger
{
public:
	static Share<CompilerValue> Reset(Share<CompilerValue> value, HazeVariableType type)
	{
		value->m_Type = type;
		return value;
	}
};