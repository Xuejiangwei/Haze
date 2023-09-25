#pragma once

#include "ASTBase.h"

class ASTFunctionDefine;
class ASTClassDefine;

class ASTLibrary
{
public:
	ASTLibrary(HazeCompiler* compiler, /*const SourceLocation& Location,*/ HAZE_STRING& name, HazeLibraryType type,
		std::vector<std::unique_ptr<ASTFunctionDefine>>& functionExpressions,
		std::vector<std::unique_ptr<ASTClassDefine>>& classExpressions);

	~ASTLibrary();

	void CodeGen();

private:
	HazeCompiler* m_Compiler;
	HAZE_STRING m_Name;
	HazeLibraryType m_Type;
	std::vector<std::unique_ptr<ASTFunctionDefine>> m_FunctionExpressions;
	std::vector<std::unique_ptr<ASTClassDefine>> m_ClassExpressions;
};