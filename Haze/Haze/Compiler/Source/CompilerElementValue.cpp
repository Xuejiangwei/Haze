#include "HazePch.h"
#include "CompilerElementValue.h"
#include "CompilerArrayValue.h"
#include "CompilerHashValue.h"
#include "CompilerClassValue.h"
#include "CompilerClass.h"
#include "CompilerModule.h"
#include "Compiler.h"

CompilerElementValue::CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, Share<CompilerValue> element)
	: CompilerValue(compilerModule, element->GetVariableType(), HazeVariableScope::Local, HazeDataDesc::Element, 0), m_Parent(parent), m_Element(element), m_ElementName(nullptr)
{
	if (parent->IsArray())
	{
		m_Type = DynamicCast<CompilerArrayValue>(parent)->GetElementType();
	}
	else if (parent->IsHash())
	{
		m_Type = DynamicCast<CompilerHashValue>(parent)->GetValueType();
	}
}

CompilerElementValue::CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, const HString& elementName)
	: CompilerValue(compilerModule, HAZE_VAR_TYPE(HazeValueType::DynamicClassUnknow), HazeVariableScope::Local, HazeDataDesc::Element, 0),
	m_Parent(parent), m_Element(nullptr), m_ElementName(MakeUnique<HString>(elementName))
{

}

CompilerElementValue::~CompilerElementValue()
{
}

Share<CompilerValue> CompilerElementValue::CreateGetFunctionCall()
{
	/*if (IsArrayType(GetParentBaseType()))
	{
		return m_Module->GetCompiler()->CreateGetArrayElement(m_Parent, m_Element);
	}
	else if (IsClassType(GetParentBaseType()))
	{
		return  m_Module->GetCompiler()->CreateGetClassMember(m_Parent, m_Element);
	}
	else if (IsDynamicClassType(GetParentBaseType()))
	{
		return m_Module->GetCompiler()->CreateGetDynamicClassMember(m_Parent, *m_ElementName.get());
	}
	else
	{
		COMPILER_ERR_MODULE_W("复杂类型<%s>不支持<%s>方法", GetHazeValueTypeString(GetParentBaseType()), HAZE_ADVANCE_GET_FUNCTION,
			m_Module->GetName().c_str());
	}*/

	return m_Module->GetCompiler()->CreateGetAdvanceElement(DynamicCast<CompilerElementValue>(GetShared()));
}

CompilerClass* CompilerElementValue::GetRealClass() const
{
	if (m_Element)
	{
		if (m_Element->IsElement())
		{
			return DynamicCast<CompilerElementValue>(m_Element)->GetRealClass();
		}
		
		return DynamicCast<CompilerClassValue>(m_Element)->GetOwnerClass();
	}
	
	return nullptr;
}
