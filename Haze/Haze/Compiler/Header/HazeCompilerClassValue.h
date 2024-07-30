#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerClass;

class HazeCompilerClassValue : public HazeCompilerValue
{
public:
	friend class HazeCompilerClass;

	explicit HazeCompilerClassValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count);

	virtual ~HazeCompilerClassValue() override;

	virtual uint32 GetSize() override;

	HazeCompilerClass* GetOwnerClass() { return m_OwnerClass; };

	const HString& GetOwnerClassName();

	Share<HazeCompilerValue> GetMember(const HString& name);

	void GetMemberName(const HazeCompilerValue* memberValue, HString& outName, bool getOffset = false, V_Array<uint64>* offsets = nullptr);

private:
	HazeCompilerClass* m_OwnerClass;

	V_Array<Pair<HazeDataDesc, V_Array<Share<HazeCompilerValue>>>> m_Data;
};
