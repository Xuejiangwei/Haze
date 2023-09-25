#pragma once

#include "ASTBase.h"

class ASTFunctionDefine;
class ASTClassDefine;

class ASTLibrary
{
public:
	ASTLibrary(HazeCompiler* m_Compiler, /*const SourceLocation& Location,*/ HAZE_STRING& m_Name, HazeLibraryType Type,
		std::vector<std::unique_ptr<ASTFunctionDefine>>& Vector_FunctionExpression, std::vector<std::unique_ptr<ASTClassDefine>>& Vector_ClassExpression);
	~ASTLibrary();

	void CodeGen();

private:
	HazeCompiler* m_Compiler;
	HAZE_STRING m_Name;
	HazeLibraryType Type;
	std::vector<std::unique_ptr<ASTFunctionDefine>> Vector_FunctionExpression;
	std::vector<std::unique_ptr<ASTClassDefine>> Vector_ClassExpression;
};