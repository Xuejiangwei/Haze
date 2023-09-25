#pragma once

#include "HazeCompilerPointerValue.h"

class HazeCompilerPointerFunction : public HazeCompilerPointerValue
{
public:
	explicit HazeCompilerPointerFunction(HazeCompilerModule* m_Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count, std::vector<HazeDefineType> Vector_ParamType);

	virtual ~HazeCompilerPointerFunction() override;

	const std::vector<HazeDefineType>& GetParamType() const { return Vector_ParamType; }

private:
	std::vector<HazeDefineType> Vector_ParamType;
};
