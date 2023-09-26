#pragma once

#include "HazeCompilerPointerValue.h"

class HazeCompilerPointerFunction : public HazeCompilerPointerValue
{
public:
	explicit HazeCompilerPointerFunction(HazeCompilerModule* compilerModule, const HazeDefineType& defineType,
		HazeVariableScope scope, HazeDataDesc desc, int count, std::vector<HazeDefineType>* paramTypes);

	virtual ~HazeCompilerPointerFunction() override;

	const std::vector<HazeDefineType>& GetParamTypes() const { return m_ParamTypes; }

private:
	std::vector<HazeDefineType> m_ParamTypes;
};
