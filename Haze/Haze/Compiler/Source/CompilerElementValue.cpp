#include "HazePch.h"
#include "CompilerElementValue.h"
#include "CompilerArrayValue.h"
#include "CompilerHashValue.h"
#include "CompilerClassValue.h"
#include "CompilerClass.h"
#include "CompilerModule.h"
#include "Compiler.h"

CompilerElementValue::CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, Share<CompilerValue> element)
	: CompilerValue(compilerModule, element->GetValueType(), HazeVariableScope::Local, HazeDataDesc::Element, 0), m_Parent(parent), m_Element(element), m_ElementName(nullptr),
	m_ValueTypeArrayDimension(0)
{
	if (parent->IsArray())
	{
		auto arrayValue = DynamicCast<CompilerArrayValue>(parent);
		if (arrayValue->GetArrayDimension() > 1)
		{
			m_ValueType = parent->GetValueType();
			m_ValueTypeArrayDimension = arrayValue->GetArrayDimension() - 1;
		}
		else
		{
			m_ValueType.ToArrayElement(parent->GetValueType());
		}
	}
	else if (parent->IsHash())
	{
		auto& type = DynamicCast<CompilerHashValue>(parent)->GetValueType().Type;
		m_ValueType = type->BaseType;
		m_ValueTypeArrayDimension = type->ArrayDimension;
	}
}

CompilerElementValue::CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, const HString& elementName)
	: CompilerValue(compilerModule, HazeValueType::DynamicClassUnknow, HazeVariableScope::Local, HazeDataDesc::Element, 0), m_Parent(parent), m_Element(nullptr), m_ElementName(MakeUnique<HString>(elementName))
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
		COMPILER_ERR_MODULE_W("��������<%s>��֧��<%s>����", GetHazeValueTypeString(GetParentBaseType()), HAZE_ADVANCE_GET_FUNCTION,
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
