#pragma once

class CompilerClass;

class HazeCompilerTemplateClass
{
public:
	HazeCompilerTemplateClass();
	
	~HazeCompilerTemplateClass();

private:
	V_Array<Share<CompilerClass>> m_GenerateClass;
};
