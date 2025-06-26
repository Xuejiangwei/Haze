#pragma once

#include "HazeCompilerPointerValue.h"

class CompilerPointerFunction : public HazeCompilerPointerValue
{
	friend class CompilerModule;
public:
	explicit CompilerPointerFunction(CompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count, TemplateDefineTypes* params);

	virtual ~CompilerPointerFunction() override;

	//const V_Array<HazeDefineType>& GetParamTypes() const { return m_ParamTypes; }

	const HazeDefineType& GetFunctionType() const { return m_FuncType.Type->BaseType; }

	const HazeDefineType& GetParamTypeByIndex(x_uint64 index) const;

	const HazeDefineType& GetParamTypeLeftToRightByIndex(x_uint64 index) const;

	const x_uint64 GetParamSize() const;

private:
	V_Array<TemplateDefineType> m_ParamTypes;
	TemplateDefineType m_FuncType;
	CompilerClass* m_OwnerClass;
};
