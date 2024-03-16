#pragma once

#include "HazeHeader.h"

class ASTBase;
class ASTClassFunctionSection;
class ASTFunctionDefine;

class ASTClass
{
public:
	ASTClass(HazeCompiler* compiler, /*const SourceLocation& Location,*/ HAZE_STRING& name, std::vector<HAZE_STRING>& parentClass,
		std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>>& data, std::unique_ptr<ASTClassFunctionSection>& functionSection);
	
	~ASTClass();

	void CodeGen();
private:
	HazeCompiler* m_Compiler;

	HAZE_STRING m_ClassName;
	std::vector<HAZE_STRING> m_ParentClasses;
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> m_ClassDatas;
	std::unique_ptr<ASTClassFunctionSection> m_ClassFunctionSection;
};

class ASTClassDefine
{
public:
	ASTClassDefine(HazeCompiler* compiler, /*const SourceLocation& Location,*/ HAZE_STRING& name, 
		std::vector<std::vector<std::unique_ptr<ASTBase>>>& data, std::vector<std::unique_ptr<ASTFunctionDefine>>& function);

	~ASTClassDefine();

	void CodeGen();
private:
	HazeCompiler* m_Compiler;

	HAZE_STRING m_ClassName;
	std::vector<std::vector<std::unique_ptr<ASTBase>>> m_ClassDatas;
	std::vector<std::unique_ptr<ASTFunctionDefine>> m_ClassFunctions;
};
