#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerEnum;

class HazeCompilerEnumValue : public HazeCompilerValue
{
public:
	friend class HazeCompilerClass;

	explicit HazeCompilerEnumValue(HazeCompilerEnum* owner, Share<HazeCompilerValue> value);

	explicit HazeCompilerEnumValue(HazeCompilerEnum* owner, HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, Share<HazeCompilerValue> assignValue = nullptr);

	virtual ~HazeCompilerEnumValue() override;

	HazeCompilerEnum* GetEnum() { return m_OwnerEnum; }

	HazeValueType GetBaseType() const;

	virtual bool IsEnum() const { return true; }

	virtual uint32 GetSize();

private:
	HazeCompilerEnum* m_OwnerEnum;
};
