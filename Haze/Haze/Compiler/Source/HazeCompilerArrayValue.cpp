#include "HazeCompilerArrayValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"

HazeCompilerArrayElementValue::HazeCompilerArrayElementValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, 
	HazeCompilerValue* Array, HazeCompilerValue* Index) : HazeCompilerValue(Module, DefineType, Scope, Count), Array(Array), Index(Index)
{
}

HazeCompilerArrayElementValue::~HazeCompilerArrayElementValue()
{
}

HazeCompilerArrayValue::HazeCompilerArrayValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, HazeCompilerValue* ArraySize)
	: HazeCompilerValue(Module, DefineType, Scope, Count), ArrayType(DefineType)
{
	ArrayLength = ArraySize->GetValue().Type == HazeValueType::UnsignedLong || ArraySize->GetValue().Type == HazeValueType::Long ?
		(uint32)ArraySize->GetValue().Value.UnsignedLong : ArraySize->GetValue().Value.UnsignedInt;

	Size = ArrayLength * GetSizeByType(DefineType, Module);
}

HazeCompilerArrayValue::~HazeCompilerArrayValue()
{
}
