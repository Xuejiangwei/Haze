#include "HazeCompilerValue.h"
#include "HazeCompilerModule.h"

#include <string.h>

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module) : Module(Module)
{
	Value.Value.LongValue = 0;
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, HazeValueType Type) : Module(Module)
{
	Value.Type = Type;
	Value.Value.LongValue = 0;
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, HazeValue Value) : Module(Module), Value(Value)
{
}

HazeCompilerValue::~HazeCompilerValue()
{
}

void HazeCompilerValue::StoreValue(HazeCompilerValue* Value)
{
	memcpy(&this->Value.Value, &Value->Value.Value, sizeof(this->Value.Value));

	//Éú³É×Ö½ÚÂë
}