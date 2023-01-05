#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;



class HazeCompilerValue
{
public:
	enum class ValueSection : int8_t
	{
		Global,
		Local,
	};

public:
	HazeCompilerValue();

	//HazeCompilerValue(HazeCompilerModule* Module);

	//HazeCompilerValue(HazeCompilerModule* Module, HazeValueType Type);

	HazeCompilerValue(HazeValue Value);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, ValueSection Section, int Address = -1);

	~HazeCompilerValue();

	void StoreValue(std::shared_ptr<HazeCompilerValue> Value);

	int GetAddress() { return Address; }

	HazeValue& GetValue() { return Value; }

	bool IsConstant() { return !Module; }

	bool IsGlobal() { return Section == ValueSection::Global; }

private:
	HazeValue Value;
	HazeCompilerModule* Module;
	ValueSection Section;
	int Address;
};
