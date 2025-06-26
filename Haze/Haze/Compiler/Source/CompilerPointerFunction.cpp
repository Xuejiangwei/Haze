#include "HazePch.h"
#include "CompilerPointerFunction.h"
#include "CompilerModule.h"
#include "HazeLogDefine.h"

CompilerPointerFunction::CompilerPointerFunction(CompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, TemplateDefineTypes* paramTypes)
	: HazeCompilerPointerValue(compilerModule, defineType, scope, desc, count), m_OwnerClass(nullptr)
{
	if (paramTypes)
	{
		m_ParamTypes = Move(paramTypes->Types);
		m_FuncType = m_ParamTypes[0];
		m_ParamTypes.erase(m_ParamTypes.begin());
	}
}

CompilerPointerFunction::~CompilerPointerFunction()
{
}

const HazeDefineType& CompilerPointerFunction::GetParamTypeByIndex(x_uint64 index) const
{
	if (index + 1 < m_ParamTypes.size())
	{
		return m_ParamTypes[m_ParamTypes.size() - 1 - index].Type->BaseType;
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("�������󣬻��ָ�뺯���ĵ�<%d>����������", index);
			return m_ParamTypes[0].Type->BaseType;
		}
		else
		{
			return m_ParamTypes[0].Type->BaseType;
		}
	}
}

const HazeDefineType& CompilerPointerFunction::GetParamTypeLeftToRightByIndex(x_uint64 index) const
{
	if (index < m_ParamTypes.size())
	{
		return m_ParamTypes[index].Type->BaseType;
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("�������ң����ָ�뺯���ĵ�<%d>����������", m_ParamTypes.size() - 1 - index);
		}

		return m_ParamTypes[0].Type->BaseType;
	}
}

const x_uint64 CompilerPointerFunction::GetParamSize() const
{
	return m_OwnerClass ? m_ParamTypes.size() - 1 : m_ParamTypes.size();
}