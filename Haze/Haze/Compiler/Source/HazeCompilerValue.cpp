#include "HazeCompilerValue.h"
#include "HazeCompilerModule.h"

#include <string.h>

HazeCompilerValue::HazeCompilerValue() : Module(nullptr), Section(HazeCompilerValue::ValueSection::Local), Address(-1)
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

HazeCompilerValue::HazeCompilerValue(HazeValue Value) : Module(nullptr), Value(Value), Section(HazeCompilerValue::ValueSection::Local), Address(-1)
{
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeCompilerValue::ValueSection Section, int Address) : Module(Module), Section(Section), Address(Address)
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


