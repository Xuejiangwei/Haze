#include "HazeCompilerPointerArray.h"

HazeCompilerPointerArray::HazeCompilerPointerArray(HazeCompilerModule* m_Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize)
	:HazeCompilerPointerValue(m_Module, DefineType, Scope, Desc, Count), Vector_ArraySize(m_ArraySize)
{
}

HazeCompilerPointerArray::~HazeCompilerPointerArray()
{
}

uint32 HazeCompilerPointerArray::GetSizeByLevel(uint32 m_Level)
{
	uint32 Ret = 0;
	if (m_Level < Vector_ArraySize.size())
	{
		Ret = 1;
		for (size_t i = m_Level; i < Vector_ArraySize.size(); i++)
		{
			Ret *= Vector_ArraySize[i]->GetValueType().PrimaryType == HazeValueType::UnsignedLong || Vector_ArraySize[i]->GetValueType().PrimaryType == HazeValueType::Long ?
				(uint32)Vector_ArraySize[i]->GetValue().Value.UnsignedLong : Vector_ArraySize[i]->GetValue().Value.UnsignedInt;
		}
	}

	return Ret;
}