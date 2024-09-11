#include "HazePch.h"
#include "CompilerElementValue.h"
#include "CompilerArrayValue.h"
#include "CompilerModule.h"
#include "Compiler.h"

CompilerElementValue::CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, Share<CompilerValue> element)
	: CompilerValue(compilerModule, element->GetValueType(), HazeVariableScope::Local, HazeDataDesc::Element, 0), m_Parent(parent), m_Element(element)
{
	if (parent->IsArray())
	{
		auto arrayValue = DynamicCast<CompilerArrayValue>(parent);
		if (arrayValue->GetArrayDimension() > 1)
		{
			m_ValueType = parent->GetValueType();
		}
		else
		{
			m_ValueType.ToArrayElement(parent->GetValueType());
		}
	}
}

CompilerElementValue::~CompilerElementValue()
{
}

Share<CompilerValue> CompilerElementValue::CreateGetFunctionCall()
{
	if (IsArrayType(GetParentBaseType()))
	{
		return m_Module->GetCompiler()->CreateGetArrayElement(m_Parent, m_Element);
	}
	else if (IsClassType(GetParentBaseType()))
	{
		return  m_Module->GetCompiler()->CreateGetClassMember(m_Parent, m_Element);
	}
	else
	{
		COMPILER_ERR_MODULE_W("复杂类型<%s>不支持<%s>方法", GetHazeValueTypeString(GetParentBaseType()), HAZE_ADVANCE_GET_FUNCTION,
			m_Module->GetName().c_str());
	}

	return nullptr;
}
