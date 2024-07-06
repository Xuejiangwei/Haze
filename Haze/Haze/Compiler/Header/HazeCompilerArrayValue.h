#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerArrayElementValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerArrayElementValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, HazeCompilerValue* arrayValue, V_Array<HazeCompilerValue*> index);

	virtual ~HazeCompilerArrayElementValue() override;

	HazeCompilerValue* GetArray() const { return m_ArrayOrPointer; }

	const V_Array<HazeCompilerValue*>& GetIndex() const { return m_ArrayIndex; }

private:
	HazeCompilerValue* m_ArrayOrPointer;			//数组或者指针
	V_Array<HazeCompilerValue*> m_ArrayIndex;
};

class HazeCompilerArrayValue : public HazeCompilerValue
{
public:
	explicit HazeCompilerArrayValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope, 
		HazeDataDesc desc, int count, V_Array<Share<HazeCompilerValue>>& arraySize);

	virtual ~HazeCompilerArrayValue() override;

	//virtual uint32 GetSize() override { return m_Size; }

	uint32 GetArrayLength() { return m_ArrayLength; }

	const V_Array<Share<HazeCompilerValue>>& GetArraySize() const { return m_SizeValues; }

	uint32 GetSizeByLevel(uint32 level);

private:
	uint32 m_ArrayLength;
	uint32 m_Size;

	V_Array<Share<HazeCompilerValue>> m_SizeValues;
};
