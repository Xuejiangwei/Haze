#include "HazeCompilerValue.h"
#include "HazeCompilerModule.h"

#include <string.h>

HazeCompilerValue::HazeCompilerValue() : Module(nullptr), Scope(InstructionScopeType::Local)
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

HazeCompilerValue::HazeCompilerValue(HazeValue Value, InstructionScopeType Scope) : Module(nullptr), Value(Value), Scope(Scope)
{
}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, InstructionScopeType Scope) 
	: Module(Module), Scope(Scope)
{
	Value.Type = DefineType.PrimaryType;
	memset(&Value.Value, 0, sizeof(Value.Value));
}

HazeCompilerValue::~HazeCompilerValue()
{
}

void HazeCompilerValue::StoreValue(std::shared_ptr<HazeCompilerValue> Value)
{
	memcpy(&this->Value.Value, &Value->Value.Value, sizeof(this->Value.Value));
}

unsigned int HazeCompilerValue::GetSize()
{
	return GetSizeByHazeType(Value.Type);
}



