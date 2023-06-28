#pragma once

#include "ASTBase.h"

class ASTFunctionDefine;
class ASTClassDefine;

class ASTLibrary
{
public:
	ASTLibrary(HazeCompiler* Compiler, /*const SourceLocation& Location,*/ HAZE_STRING& Name, HazeLibraryType Type,
		std::vector<std::unique_ptr<ASTFunctionDefine>>& Vector_FunctionExpression, std::vector<std::unique_ptr<ASTClassDefine>>& Vector_ClassExpression);
	~ASTLibrary();

	void CodeGen();

private:
	HazeCompiler* Compiler;
	HAZE_STRING Name;
	HazeLibraryType Type;
	std::vector<std::unique_ptr<ASTFunctionDefine>> Vector_FunctionExpression;
	std::vector<std::unique_ptr<ASTClassDefine>> Vector_ClassExpression;
};