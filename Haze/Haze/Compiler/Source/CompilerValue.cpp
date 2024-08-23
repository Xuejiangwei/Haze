#include "HazePch.h"
#include "CompilerValue.h"
#include "CompilerModule.h"
#include "CompilerHelper.h"

#include <string.h>

//HazeCompilerValue::HazeCompilerValue() : Module(nullptr), Scope(HazeDataDesc::Local), Count(0)
//{
//	memset(&Value, 0, sizeof(Value));
//}

//HazeCompilerValue::HazeCompilerValue(HazeValue Value, HazeDataDesc Scope) : Module(nullptr), Value(Value), Scope(Scope), Count(0)
//{
//}

CompilerValue::CompilerValue(CompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
	HazeDataDesc desc, int count, Share<CompilerValue> AssignValue)
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

CompilerValue::~CompilerValue()
{
}

void CompilerValue::StoreValueType(Share<CompilerValue> srcValue)
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

uint32 CompilerValue::GetSize()
{
	return GetSizeByHazeType(m_ValueType.PrimaryType);
}

bool CompilerValue::TryGetVariableName(HString& outName)
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