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

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count) 
	: Module(Module), Scope(Scope), Count(Count)
{
	Value.Type = DefineType.PrimaryType;
	memset(&Value.Value, 0, sizeof(Value.Value));
}

HazeCompilerValue::~HazeCompilerValue()
{
}

void HazeCompilerValue::StoreValue(std::shared_ptr<HazeCompilerValue> SrcValue)
{
	memcpy(&this->Value, &SrcValue->Value, sizeof(this->Value));
}

uint32 HazeCompilerValue::GetSize()
{
	return GetSizeByHazeType(Value.Type);
}



