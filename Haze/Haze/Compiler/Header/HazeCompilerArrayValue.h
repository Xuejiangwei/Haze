#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerArrayElementValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerArrayElementValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, HazeCompilerValue* Array, 
		std::vector<HazeCompilerValue*> Index);
	
	~HazeCompilerArrayElementValue() override;

	HazeCompilerValue* GetArray() const { return ArrayOrPointer; }

	const std::vector<HazeCompilerValue*>& GetIndex() const { return Index; }

private:
	HazeCompilerValue* ArrayOrPointer; //数组或者指针
	std::vector<HazeCompilerValue*> Index;
};

class HazeCompilerArrayValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerArrayValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, 
		std::vector<std::shared_ptr<HazeCompilerValue>>& ArraySize);
	~HazeCompilerArrayValue();

	virtual uint32 GetSize() override { return Size; }

	uint32 GetArrayLength() { return ArrayLength; }

	const std::vector<HazeCompilerValue*>& GetSizeValue() const { return Vector_Size; }

private:
	uint32 ArrayLength;
	uint32 Size;

	std::vector<HazeCompilerValue*> Vector_Size;
};

