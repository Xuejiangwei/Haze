#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerModule;



class HazeCompilerValue
{
public:
	enum class ValueSection : int8_t
	{
		Register,
		Constant,
		Global,
		Local,
	};

public:
	HazeCompilerValue();

	//HazeCompilerValue(HazeCompilerModule* Module);

	//HazeCompilerValue(HazeCompilerModule* Module, HazeValueType Type);

	HazeCompilerValue(HazeValue Value, ValueSection Section);

	HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, ValueSection Section);

	~HazeCompilerValue();

	void StoreValue(std::shared_ptr<HazeCompilerValue> Value);

	HazeValue& GetValue() { return Value; }

	bool IsRegister() { return Section == ValueSection::Register; }

	bool IsConstant() { return Section == ValueSection::Constant; }

	bool IsGlobal() { return Section == ValueSection::Global; }

	bool IsLocal() { return Section == ValueSection::Local; }

private:
	HazeValue Value;
	HazeCompilerModule* Module;
	ValueSection Section;
};
