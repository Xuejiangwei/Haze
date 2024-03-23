#pragma once
#include "HazeHeader.h"

class HazeCompilerClass;

class HazeCompilerTemplateClass
{
public:
	HazeCompilerTemplateClass();
	
	~HazeCompilerTemplateClass();

private:
	std::vector<std::shared_ptr<HazeCompilerClass>> m_GenerateClass;
};
