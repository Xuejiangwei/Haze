#include "HazePch.h"
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

HazeCompilerValue::HazeCompilerValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
	HazeDataDesc desc, int count, Share<HazeCompilerValue> AssignValue)
	: m_Module(compilerModule), m_ValueType(defineType), m_Scope(scope), m_Desc(desc), m_Count(count)
{
	if (AssignValue)
	{
		memcpy(&m_Value.Value, &AssignValue->GetValue(), sizeof(m_Value.Value));
	}
	else
	{
		memset(&m_Value.Value, 0, sizeof(m_Value.Value));
	}
}

HazeCompilerValue::~HazeCompilerValue()
{
}

void HazeCompilerValue::StoreValueType(Share<HazeCompilerValue> srcValue)
{
	if (IsRegister())
	{
		bool bPointer = false;// IsPointer();
		m_ValueType = srcValue->GetValueType();

		if (srcValue->IsArray() && bPointer)
		{
			//m_ValueType.PrimaryType = m_ValueType.CustomName.empty() ? HazeValueType::PointerBase : HazeValueType::PointerClass;
		}
	}

	//memcpy(&this->Value.Value, &SrcValue->Value.Value, sizeof(this->Value.Value));
}

uint32 HazeCompilerValue::GetSize()
{
	return GetSizeByHazeType(m_ValueType.PrimaryType);
}

bool HazeCompilerValue::TryGetVariableName(HString& outName)
{
	bool ret = false;
	HAZE_STRING_STREAM hss;
	if (IsConstant())
	{
		HazeCompilerStream(hss, this);
		outName = hss.str();
		ret = true;
	}

	return ret;
}