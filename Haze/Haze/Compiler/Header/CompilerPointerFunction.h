#pragma once

#include "CompilerValue.h"

class CompilerPointerFunction : public CompilerValue
{
	friend class CompilerModule;
public:
	explicit CompilerPointerFunction(CompilerModule* compilerModule, const HazeVariableType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count, TemplateDefineTypes* params);

	virtual ~CompilerPointerFunction() override;

	//const V_Array<HazeDefineType>& GetParamTypes() const { return m_ParamTypes; }

	const HazeVariableType& GetFunctionType() const { return m_FuncType.Type->BaseType; }

	const HazeVariableType& GetParamTypeByIndex(x_uint64 index) const;

	const HazeVariableType& GetParamTypeLeftToRightByIndex(x_uint64 index) const;

	const x_uint64 GetParamCount() const;

private:
	V_Array<TemplateDefineType> m_ParamTypes;
	TemplateDefineType m_FuncType;
	CompilerClass* m_OwnerClass;
};
