#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerEnum;

class HazeCompilerEnumValue : public HazeCompilerValue
{
public:
	friend class HazeCompilerClass;

	explicit HazeCompilerEnumValue(HazeCompilerEnum* owner, Share<HazeCompilerValue>& value);

	virtual ~HazeCompilerEnumValue() override;

private:
	HazeCompilerEnum* m_OwnerEnum;
};
