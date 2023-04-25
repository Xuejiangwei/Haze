#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerArrayElementValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerArrayElementValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, HazeCompilerValue* Array, HazeCompilerValue* Index);
	~HazeCompilerArrayElementValue();

	HazeCompilerValue* GetArray() const { return Array; }

	HazeCompilerValue* GetIndex() const { return Index; }

private:
	HazeCompilerValue* Array;
	HazeCompilerValue* Index;
};

class HazeCompilerArrayValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerArrayValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, HazeCompilerValue* ArraySize);
	~HazeCompilerArrayValue();

	const HazeDefineType& GetArrayType() const { return ArrayType; }

	virtual uint32 GetSize() override { return Size; }

	uint32 GetArrayLength() { return ArrayLength; }

private:
	HazeDefineType ArrayType;

	uint32 ArrayLength;
	uint32 Size;
};

