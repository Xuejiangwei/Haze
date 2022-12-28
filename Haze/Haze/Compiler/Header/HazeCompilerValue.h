#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;

class HazeCompilerValue
{
public:
	HazeCompilerValue();

	HazeCompilerValue(HazeCompilerModule* Module);

	HazeCompilerValue(HazeCompilerModule* Module, HazeValueType Type);

	HazeCompilerValue(HazeValue Value);

	HazeCompilerValue(HazeCompilerModule* Module, HazeValue Value);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType);

	~HazeCompilerValue();

	void StoreValue(std::shared_ptr<HazeCompilerValue> Value);

	HazeValue& GetValue() { return Value; }

private:
	HazeValue Value;
	HazeCompilerModule* Module;
};
