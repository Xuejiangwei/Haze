#pragma once

class HazeCompilerClass;

class HazeCompilerTemplateClass
{
public:
	HazeCompilerTemplateClass();
	
	~HazeCompilerTemplateClass();

private:
	V_Array<Share<HazeCompilerClass>> m_GenerateClass;
};
