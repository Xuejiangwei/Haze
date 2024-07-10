#pragma once

#include "HazeCompilerPointerValue.h"

class HazeCompilerPointerFunction : public HazeCompilerPointerValue
{
public:
	explicit HazeCompilerPointerFunction(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count, V_Array<HazeDefineType>* paramTypes);

	virtual ~HazeCompilerPointerFunction() override;

	const V_Array<HazeDefineType>& GetParamTypes() const { return m_ParamTypes; }

	const HazeDefineType& GetParamTypeByIndex(int index) const;

private:
	V_Array<HazeDefineType> m_ParamTypes;
};
