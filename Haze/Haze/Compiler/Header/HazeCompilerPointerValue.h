#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerPointerValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerPointerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, std::shared_ptr<HazeCompilerValue> RefValue = nullptr);

	virtual ~HazeCompilerPointerValue() override;

	//virtual void StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue) override;

	//void InitPointerTo(HazeCompilerValue* PointerToValue);

	//void InitPointerTo(std::shared_ptr<HazeCompilerValue> PointerToValue);

	//std::shared_ptr<HazeCompilerValue> GetPointerValue() const;

private:
	std::shared_ptr<HazeCompilerValue> RefValue;
};