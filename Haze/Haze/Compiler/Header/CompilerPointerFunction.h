#pragma once

#include "HazeCompilerPointerValue.h"

class CompilerPointerFunction : public HazeCompilerPointerValue
{
	friend class CompilerModule;
public:
	explicit CompilerPointerFunction(CompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count, V_Array<HazeDefineType>* paramTypes);

	virtual ~CompilerPointerFunction() override;

	const V_Array<HazeDefineType>& GetParamTypes() const { return m_ParamTypes; }

	const HazeDefineType& GetParamTypeByIndex(uint64 index) const;

	const HazeDefineType& GetParamTypeLeftToRightByIndex(uint64 index) const;

private:
	V_Array<HazeDefineType> m_ParamTypes;
};
