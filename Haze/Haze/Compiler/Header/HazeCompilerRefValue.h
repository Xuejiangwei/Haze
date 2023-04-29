#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerRefValue : public HazeCompilerValue
{
public:
	HazeCompilerRefValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, std::shared_ptr<HazeCompilerValue>& RefValue);
	~HazeCompilerRefValue();

	virtual void StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue) override;

	const std::shared_ptr<HazeCompilerValue>& GetRefValue() const { return RefValue; }

private:
	std::shared_ptr<HazeCompilerValue> RefValue;
};
