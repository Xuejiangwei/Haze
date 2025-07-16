#include "HazePch.h"
#include "CompilerPointerFunction.h"
#include "CompilerModule.h"
#include "HazeLogDefine.h"

CompilerPointerFunction::CompilerPointerFunction(CompilerModule* compilerModule, const HazeVariableType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, TemplateDefineTypes* paramTypes)
	: CompilerValue(compilerModule, defineType, scope, desc, count), m_OwnerClass(nullptr)
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

const HazeVariableType& CompilerPointerFunction::GetParamTypeByIndex(x_uint64 index) const
{
	if (index + 1 < m_ParamTypes.size())
	{
		return m_ParamTypes[m_ParamTypes.size() - 1 - index].Type->BaseType;
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("从右往左，获得指针函数的第<%d>个参数错误", index);
			return m_ParamTypes[0].Type->BaseType;
		}
		else
		{
			return m_ParamTypes[0].Type->BaseType;
		}
	}
}

const HazeVariableType& CompilerPointerFunction::GetParamTypeLeftToRightByIndex(x_uint64 index) const
{
	if (index < m_ParamTypes.size())
	{
		return m_ParamTypes[index].Type->BaseType;
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("从左往右，获得指针函数的第<%d>个参数错误", m_ParamTypes.size() - 1 - index);
		}

		return m_ParamTypes[0].Type->BaseType;
	}
}

const x_uint64 CompilerPointerFunction::GetParamSize() const
{
	return m_OwnerClass ? m_ParamTypes.size() - 1 : m_ParamTypes.size();
}