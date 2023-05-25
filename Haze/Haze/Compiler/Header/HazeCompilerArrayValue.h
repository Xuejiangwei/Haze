#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerArrayElementValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerArrayElementValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, HazeCompilerValue* Array, 
		std::vector<HazeCompilerValue*> Index);
	
	virtual ~HazeCompilerArrayElementValue() override;

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
	
	virtual ~HazeCompilerArrayValue() override;

	virtual uint32 GetSize() override { return Size; }

	uint32 GetArrayLength() { return ArrayLength; }

	const std::vector<std::shared_ptr<HazeCompilerValue>>& GetArraySize() const { return Vector_Size; }

	uint32 GetSizeByLevel(uint32 Level);

private:
	uint32 ArrayLength;
	uint32 Size;

	std::vector<std::shared_ptr<HazeCompilerValue>> Vector_Size;
};

