#include "HazeCompilerValue.h"
#include "HazeCompilerModule.h"

#include <string.h>

HazeCompilerValue::HazeCompilerValue() : Module(nullptr), Scope(HazeDataDesc::Local)
{
	memset(&Value, 0, sizeof(Value));
}

/*HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module) : Module(Module), Address(-1)
{
	memset(&Value.Value, 0, sizeof(Value.Value));
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, HazeValueType Type) : Module(Module), Address(-1)
{
	Value.Type = Type;
	memset(&Value.Value, 0, sizeof(Value.Value));
}*/

HazeCompilerValue::HazeCompilerValue(HazeValue Value, HazeDataDesc Scope) : Module(nullptr), Value(Value), Scope(Scope), Count(0)
{
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count, HazeValue* DefaultValue)
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
		ValueType = SrcValue->GetValueType();
	}

	memcpy(&this->Value.Value, &SrcValue->Value.Value, sizeof(this->Value.Value));
}

uint32 HazeCompilerValue::GetSize()
{
	return GetSizeByHazeType(ValueType.PrimaryType);
}



