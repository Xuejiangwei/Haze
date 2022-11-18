#include "HazeCompilerValue.h"

#include <string.h>

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module) : Module(Module)
{
	Value.LongValue = 0;
}

HazeCompilerValue::~HazeCompilerValue()
{
}

void HazeCompilerValue::StoreValue(HazeCompilerValue* Value)
{
	memcpy(&this->Value, &Value->Value, sizeof(this->Value));

	//Éú³É×Ö½ÚÂë
}