#include "HazeCompilerPointerArray.h"

HazeCompilerPointerArray::HazeCompilerPointerArray(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize)
	:HazeCompilerPointerValue(Module, DefineType, Scope, Desc, Count), Vector_ArraySize(ArraySize)
{
}

HazeCompilerPointerArray::~HazeCompilerPointerArray()
{
}

uint32 HazeCompilerPointerArray::GetSizeByLevel(uint32 Level)
{
	uint32 Ret = 0;
	if (Level < Vector_ArraySize.size())
	{
		Ret = 1;
		for (size_t i = Level; i < Vector_ArraySize.size(); i++)
		{
			Ret *= Vector_ArraySize[i]->GetValueType().PrimaryType == HazeValueType::UnsignedLong || Vector_ArraySize[i]->GetValueType().PrimaryType == HazeValueType::Long ?
				(uint32)Vector_ArraySize[i]->GetValue().Value.UnsignedLong : Vector_ArraySize[i]->GetValue().Value.UnsignedInt;
		}
	}

	return Ret;
}