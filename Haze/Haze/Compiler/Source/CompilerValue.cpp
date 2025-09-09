#include "HazePch.h"
#include "CompilerValue.h"
#include "CompilerModule.h"
#include "CompilerHelper.h"

#include <string.h>

CompilerValue::CompilerValue(CompilerModule* compilerModule, const HazeVariableType& defineType, /*HazeVariableScope scope,*/
	HazeDataDesc desc, int count, Share<CompilerValue> AssignValue)
	: m_Module(compilerModule), m_Type(defineType), /*m_Scope(scope),*/ m_Desc(desc), m_Count(count)
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

const STDString* CompilerValue::GetPointerFunctionName() const
{
	return (const STDString*)m_Value.Value.Pointer;
}

void CompilerValue::SetPointerFunctionName(const STDString* name)
{
	m_Value.Value.Pointer = name;
}

bool CompilerValue::TryGetVariableName(HStringView& outName)
{
	static const x_HChar* s_prefix = H_TEXT("常量");
	static STDString s_constantName;

	bool ret = false;
	if (IsConstant())
	{
		s_constantName = GetHazeValueTypeString(GetBaseType());
		s_constantName += s_prefix;

		HAZE_STRING_STREAM hss;
		HazeCompilerStream(hss, this);
		s_constantName += hss.str();
		
		outName = s_constantName;
		ret = true;
	}

	return ret;
}

bool CompilerValue::TryGetVariableId(InstructionOpId& outId)
{
	if (IsConstant())
	{
		outId.Value = m_Value;
		return true;
	}

	return false;
}
