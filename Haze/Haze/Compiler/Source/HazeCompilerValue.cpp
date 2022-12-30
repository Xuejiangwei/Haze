#include "HazeCompilerValue.h"
#include "HazeCompilerModule.h"

#include <string.h>

HazeCompilerValue::HazeCompilerValue()
{
	Module = nullptr;
	memset(&Value, 0, sizeof(Value));
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module) : Module(Module)
{
	memset(&Value.Value, 0, sizeof(Value.Value));
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, HazeValueType Type) : Module(Module)
{
	Value.Type = Type;
	memset(&Value.Value, 0, sizeof(Value.Value));
}

HazeCompilerValue::HazeCompilerValue(HazeValue Value) : Module(nullptr), Value(Value)
{
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, HazeValue Value) : Module(Module), Value(Value)
{
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType)
{
	Value.Type = GetValueTypeByToken(DefineType.first);
	if (Value.Type == HazeValueType::Null)
	{
		Value.Type = Module->FindClass(DefineType.second);
	}
	memset(&Value.Value, 0, sizeof(Value.Value));
}

HazeCompilerValue::~HazeCompilerValue()
{
}

void HazeCompilerValue::StoreValue(std::shared_ptr<HazeCompilerValue> Value)
{
	memcpy(&this->Value.Value, &Value->Value.Value, sizeof(this->Value.Value));

	//Éú³É×Ö½ÚÂë
}


