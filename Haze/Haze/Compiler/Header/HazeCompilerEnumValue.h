#pragma once

#include "Haze.h"
#include "HazeCompilerValue.h"

class HazeCompilerEnum;

class HazeCompilerEnumValue : public HazeCompilerValue
{
public:
	friend class HazeCompilerClass;

	explicit HazeCompilerEnumValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count);

	virtual ~HazeCompilerEnumValue() override;

private:
	HazeCompilerEnum* m_OwnerEnum;
};
