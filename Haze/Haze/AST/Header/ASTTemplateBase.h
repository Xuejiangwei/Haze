#pragma once
#include "HazeHeader.h"

class ASTTemplateBase
{
public:
	ASTTemplateBase(std::vector<HAZE_STRING>& templateTypes);

	~ASTTemplateBase();

	virtual void CodeGen() {}

protected:
	std::vector<HAZE_STRING> m_TemplateTypes;
};