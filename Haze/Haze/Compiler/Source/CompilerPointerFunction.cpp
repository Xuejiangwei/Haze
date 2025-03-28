#include "HazePch.h"
#include "CompilerPointerFunction.h"
#include "CompilerModule.h"
#include "HazeLogDefine.h"

CompilerPointerFunction::CompilerPointerFunction(CompilerModule* compilerModule, const HazeDefineType& defineType,
	HazeVariableScope scope, HazeDataDesc desc, int count, V_Array<HazeDefineType>* paramTypes)
	: HazeCompilerPointerValue(compilerModule, defineType, scope, desc, count)
{
	if (paramTypes)
	{
		m_ParamTypes = Move(*paramTypes);
	}
}

CompilerPointerFunction::~CompilerPointerFunction()
{
}

const HazeDefineType& CompilerPointerFunction::GetParamTypeByIndex(x_uint64 index) const
{
	if (index < m_ParamTypes.size())
	{
		return m_ParamTypes[index];
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("�������󣬻��ָ�뺯���ĵ�<%d>����������", m_ParamTypes.size() - 1 - index);
		}

		return m_ParamTypes[0];
	}
}

const HazeDefineType& CompilerPointerFunction::GetParamTypeLeftToRightByIndex(x_uint64 index) const
{
	if (index + 1 < m_ParamTypes.size())
	{
		return m_ParamTypes[m_ParamTypes.size() - 1 - index];
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("�������ң����ָ�뺯���ĵ�<%d>����������", index);
			return m_ParamTypes[0];
		}
		else
		{
			return m_ParamTypes[0];
		}
	}
}
