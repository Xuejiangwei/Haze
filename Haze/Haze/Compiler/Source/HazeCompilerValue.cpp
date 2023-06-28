#include "HazeCompilerValue.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerHelper.h"

#include <string.h>

//HazeCompilerValue::HazeCompilerValue() : Module(nullptr), Scope(HazeDataDesc::Local), Count(0)
//{
//	memset(&Value, 0, sizeof(Value));
//}

//HazeCompilerValue::HazeCompilerValue(HazeValue Value, HazeDataDesc Scope) : Module(nullptr), Value(Value), Scope(Scope), Count(0)
//{
//}

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count,
	std::shared_ptr<HazeCompilerValue> AssignValue) : Module(Module), ValueType(DefineType), Scope(Scope), Desc(Desc), Count(Count)
{
	if (AssignValue)
	{
		memcpy(&Value.Value, &AssignValue->GetValue(), sizeof(Value.Value));
	}
	else
	{
		memset(&Value.Value, 0, sizeof(Value.Value));
	}
}

HazeCompilerValue::~HazeCompilerValue()
{
}

void HazeCompilerValue::StoreValueType(std::shared_ptr<HazeCompilerValue> SrcValue)
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

	//memcpy(&this->Value.Value, &SrcValue->Value.Value, sizeof(this->Value.Value));
}

void HazeCompilerValue::StoreValue(HazeValue& SrcValue)
{
	memcpy(&this->Value.Value, &SrcValue.Value, sizeof(this->Value.Value));
}

uint32 HazeCompilerValue::GetSize()
{
	return GetSizeByHazeType(ValueType.PrimaryType);
}

bool HazeCompilerValue::TryGetVariableName(HAZE_STRING& OutName)
{
	bool Ret = false;
	HAZE_STRING_STREAM HSS;
	if (IsConstant())
	{
		HazeCompilerStream(HSS, this);
		OutName = HSS.str();
		Ret = true;
	}

	return Ret;
}