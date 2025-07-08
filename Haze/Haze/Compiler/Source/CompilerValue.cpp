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

CompilerValue::CompilerValue(CompilerModule* compilerModule, const HazeVariableType& defineType, HazeVariableScope scope,
	HazeDataDesc desc, int count, Share<CompilerValue> AssignValue)
	: m_Module(compilerModule), m_Type(defineType), m_Scope(scope), m_Desc(desc), m_Count(count)
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