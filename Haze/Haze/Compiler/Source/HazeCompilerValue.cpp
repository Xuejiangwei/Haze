#include "HazeCompilerValue.h"
#include "HazeCompilerModule.h"

#include <string.h>

//HazeCompilerValue::HazeCompilerValue() : Module(nullptr), Scope(HazeDataDesc::Local), Count(0)
//{
//	memset(&Value, 0, sizeof(Value));
//}

//HazeCompilerValue::HazeCompilerValue(HazeValue Value, HazeDataDesc Scope) : Module(nullptr), Value(Value), Scope(Scope), Count(0)
//{
//}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, HazeValue* DefaultValue)
	: Module(Module), ValueType(DefineType), Scope(Scope), Count(Count)
{
	if (DefaultValue)
	{
		memcpy(&Value.Value, DefaultValue, sizeof(Value.Value));
	}
	else
	{
		memset(&Value.Value, 0, sizeof(Value.Value));
	}
}

HazeCompilerValue::~HazeCompilerValue()
{
}

void HazeCompilerValue::StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue)
{
	if (IsRegister())
	{
		bool bPointer = IsPointer();
		ValueType = SrcValue->GetValueType();
		
		if ((SrcValue->IsArray() || SrcValue->IsPointerArray()) && bPointer)
		{
			ValueType.PrimaryType = ValueType.CustomName.empty() ? HazeValueType::PointerBase : HazeValueType::PointerClass;
		}
	}

	memcpy(&this->Value.Value, &SrcValue->Value.Value, sizeof(this->Value.Value));
}

uint32 HazeCompilerValue::GetSize()
{
	return GetSizeByHazeType(ValueType.PrimaryType);
}



