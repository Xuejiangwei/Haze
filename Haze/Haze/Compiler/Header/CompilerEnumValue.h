#pragma once

#include "CompilerValue.h"

class CompilerEnum;

class CompilerEnumValue : public CompilerValue
{
public:
	friend class CompilerClass;

	// ���崴��
	explicit CompilerEnumValue(CompilerEnum* owner, Share<CompilerValue> value);

	// ��������
	explicit CompilerEnumValue(CompilerEnum* owner, CompilerModule* compilerModule, const HazeVariableType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, Share<CompilerValue> assignValue = nullptr);

	virtual ~CompilerEnumValue() override;

	virtual bool IsEnum() const override { return true; }

	CompilerEnum* GetEnum() { return m_OwnerEnum; }

	x_uint32 GetTypeId() const;

private:
	CompilerEnum* m_OwnerEnum;
};
