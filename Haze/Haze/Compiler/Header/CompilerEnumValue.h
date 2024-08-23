#pragma once

#include "CompilerValue.h"

class CompilerEnum;

class CompilerEnumValue : public CompilerValue
{
public:
	friend class CompilerClass;

	explicit CompilerEnumValue(CompilerEnum* owner, Share<CompilerValue> value);

	explicit CompilerEnumValue(CompilerEnum* owner, CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, Share<CompilerValue> assignValue = nullptr);

	virtual ~CompilerEnumValue() override;

	CompilerEnum* GetEnum() { return m_OwnerEnum; }

	HazeValueType GetBaseType() const;

	virtual bool IsEnum() const { return true; }

	virtual uint32 GetSize();

private:
	CompilerEnum* m_OwnerEnum;
};
