#pragma once

#include "CompilerValue.h"

class CompilerPointerFunction : public CompilerValue
{
	friend class CompilerModule;
public:
	explicit CompilerPointerFunction(CompilerModule* compilerModule, const HazeVariableType& defineType,
		/*HazeVariableScope scope, */HazeDataDesc desc, int count);

	virtual ~CompilerPointerFunction() override;

	HazeVariableType GetFunctionType() const;

	HazeVariableType GetParamTypeByIndex(x_uint64 index) const;

	HazeVariableType GetParamTypeLeftToRightByIndex(x_uint64 index) const;

	const x_uint64 GetParamCount() const;

private:
	CompilerClass* m_OwnerClass;
};
