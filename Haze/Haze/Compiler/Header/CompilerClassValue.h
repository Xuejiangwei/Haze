#pragma once

#include "CompilerValue.h"

class CompilerClass;

class CompilerClassValue : public CompilerValue
{
	friend class CompilerClass;
public:
	explicit CompilerClassValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count);

	virtual ~CompilerClassValue() override;

	virtual x_uint32 GetSize() override;

	CompilerClass* GetOwnerClass() { return m_OwnerClass; };

	const HString& GetOwnerClassName();

	Share<CompilerValue> GetMember(const HString& name, HString* nameSpace = nullptr);

	int GetMemberIndex(const HString& memberName, HString* nameSpace = nullptr);

	int GetMemberIndex(CompilerValue* value);

	bool GetMemberName(const CompilerValue* memberValue, HString& outName);

private:
	CompilerClass* m_OwnerClass;

	V_Array<Share<CompilerValue>> m_Data;
};
