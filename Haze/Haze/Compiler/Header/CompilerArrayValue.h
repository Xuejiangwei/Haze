#pragma once

#include "CompilerValue.h"

class CompilerArrayElementValue : public CompilerValue
{
public:
	explicit CompilerArrayElementValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count, CompilerValue* arrayValue, V_Array<CompilerValue*> index);

	virtual ~CompilerArrayElementValue() override;

	CompilerValue* GetArray() const { return m_ArrayOrPointer; }

	const V_Array<CompilerValue*>& GetIndex() const { return m_ArrayIndex; }

private:
	CompilerValue* m_ArrayOrPointer;			//数组或者指针
	V_Array<CompilerValue*> m_ArrayIndex;
};

class CompilerArrayValue : public CompilerValue
{
public:
	explicit CompilerArrayValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope, 
		HazeDataDesc desc, int count, V_Array<Share<CompilerValue>>& arraySize);

	virtual ~CompilerArrayValue() override;

	//virtual uint32 GetSize() override { return m_Size; }

	uint32 GetArrayLength() { return m_ArrayLength; }

	const V_Array<Share<CompilerValue>>& GetArraySize() const { return m_SizeValues; }

	uint64 GetSizeByLevel(uint64 level);

private:
	uint32 m_ArrayLength;
	uint32 m_Size;

	V_Array<Share<CompilerValue>> m_SizeValues;
};
