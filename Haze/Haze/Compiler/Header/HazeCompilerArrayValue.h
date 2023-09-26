#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerArrayElementValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerArrayElementValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, HazeCompilerValue* arrayValue, std::vector<HazeCompilerValue*> index);

	virtual ~HazeCompilerArrayElementValue() override;

	HazeCompilerValue* GetArray() const { return m_ArrayOrPointer; }

	const std::vector<HazeCompilerValue*>& GetIndex() const { return m_Index; }

private:
	HazeCompilerValue* m_ArrayOrPointer;			//数组或者指针
	std::vector<HazeCompilerValue*> m_Index;
};

class HazeCompilerArrayValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerArrayValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope, 
		HazeDataDesc desc, int count, std::vector<std::shared_ptr<HazeCompilerValue>>& arraySize);

	virtual ~HazeCompilerArrayValue() override;

	virtual uint32 GetSize() override { return m_Size; }

	uint32 GetArrayLength() { return m_ArrayLength; }

	const std::vector<std::shared_ptr<HazeCompilerValue>>& GetArraySize() const { return m_SizeValues; }

	uint32 GetSizeByLevel(uint32 level);

private:
	uint32 m_ArrayLength;
	uint32 m_Size;

	std::vector<std::shared_ptr<HazeCompilerValue>> m_SizeValues;
};
