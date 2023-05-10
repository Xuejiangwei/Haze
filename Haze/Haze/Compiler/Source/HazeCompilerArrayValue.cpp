#include "HazeCompilerArrayValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"

HazeCompilerArrayElementValue::HazeCompilerArrayElementValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, 
	HazeCompilerValue* Array, std::vector<HazeCompilerValue*> Index) : HazeCompilerValue(Module, DefineType, Scope, Count), ArrayOrPointer(Array), Index(Index)
{
}

HazeCompilerArrayElementValue::~HazeCompilerArrayElementValue()
{
}

HazeCompilerArrayValue::HazeCompilerArrayValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count,
	std::vector<std::shared_ptr<HazeCompilerValue>>& ArraySize)
	: HazeCompilerValue(Module, DefineType, Scope, Count), ArrayLength(0), Size(0)
{
	if (ArraySize.size() > 0)
	{
		ArrayLength = 1;
		for (auto& Iter : ArraySize)
		{
			ArrayLength *= Iter->GetValueType().PrimaryType == HazeValueType::UnsignedLong || Iter->GetValueType().PrimaryType == HazeValueType::Long ?
				(uint32)Iter->GetValue().Value.UnsignedLong : Iter->GetValue().Value.UnsignedInt;
			Vector_Size.push_back(Iter.get());
		}

		Size = ArrayLength * GetSizeByType(DefineType, Module);
	}
}

HazeCompilerArrayValue::~HazeCompilerArrayValue()
{
}
