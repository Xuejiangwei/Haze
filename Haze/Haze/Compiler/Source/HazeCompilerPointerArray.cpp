#include "HazeCompilerPointerArray.h"

HazeCompilerPointerArray::HazeCompilerPointerArray(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope, HazeDataDesc desc, int count, std::vector<std::shared_ptr<HazeCompilerValue>> arraySize)
	:HazeCompilerPointerValue(compilerModule, defineType, scope, desc, count), m_ArraySize(arraySize)
{
}

HazeCompilerPointerArray::~HazeCompilerPointerArray()
{
}

uint32 HazeCompilerPointerArray::GetSizeByLevel(uint32 m_Level)
{
	uint32 Ret = 0;
	if (m_Level < m_ArraySize.size())
	{
		Ret = 1;
		for (size_t i = m_Level; i < m_ArraySize.size(); i++)
		{
			Ret *= m_ArraySize[i]->GetValueType().PrimaryType == HazeValueType::UnsignedLong || m_ArraySize[i]->GetValueType().PrimaryType == HazeValueType::Long ?
				(uint32)m_ArraySize[i]->GetValue().Value.UnsignedLong : m_ArraySize[i]->GetValue().Value.UnsignedInt;
		}
	}

	return Ret;
}