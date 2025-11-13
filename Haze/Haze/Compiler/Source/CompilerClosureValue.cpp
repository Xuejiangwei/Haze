#include "HazePch.h"
#include "CompilerClosureValue.h"

CompilerClosureValue::CompilerClosureValue(CompilerModule* compilerModule, const HazeVariableType& defineType, /*HazeVariableScope scope,*/
	HazeDataDesc desc, int count)
	: CompilerPointerFunction(compilerModule, defineType, /*scope,*/ desc, count)
{
	/*m_FuncType = params->Types[0];
	if (params->Types.size() > 1)
	{
		m_ParamTypes.Types.insert(m_ParamTypes.Types.begin(), params->Types.begin() + 1, params->Types.end());
	}*/
}

CompilerClosureValue::~CompilerClosureValue()
{
}
