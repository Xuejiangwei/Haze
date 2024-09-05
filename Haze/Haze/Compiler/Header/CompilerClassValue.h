#pragma once

#include "CompilerValue.h"

class CompilerClass;

class CompilerClassValue : public CompilerValue
{
public:
	friend class CompilerClass;

	explicit CompilerClassValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count);

	virtual ~CompilerClassValue() override;

	virtual uint32 GetSize() override;

	CompilerClass* GetOwnerClass() { return m_OwnerClass; };

	const HString& GetOwnerClassName();

	Share<CompilerValue> GetMember(const HString& name);

	bool GetMemberName(const CompilerValue* memberValue, HString& outName, bool getOffset = false, V_Array<Pair<uint64, CompilerValue*>>* = nullptr);

private:
	CompilerClass* m_OwnerClass;

	V_Array<Pair<HazeDataDesc, V_Array<Share<CompilerValue>>>> m_Data;
};
