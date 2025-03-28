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

	virtual bool IsEnum() const override { return true; }

	virtual x_uint32 GetSize() override;

	CompilerEnum* GetEnum() { return m_OwnerEnum; }

	HazeValueType GetBaseType() const;

private:
	CompilerEnum* m_OwnerEnum;
};
