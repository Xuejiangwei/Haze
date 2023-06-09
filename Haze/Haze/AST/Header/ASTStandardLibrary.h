#pragma once

#include "ASTBase.h"

class ASTFunctionDefine;
class ASTClassDefine;

class ASTStandardLibrary
{
public:
	ASTStandardLibrary(HazeCompiler* Compiler, /*const SourceLocation& Location,*/ HAZE_STRING& Name, std::vector<std::unique_ptr<ASTFunctionDefine>>& Vector_FunctionExpression,
		std::vector<std::unique_ptr<ASTClassDefine>>& Vector_ClassExpression);
	~ASTStandardLibrary();

	void CodeGen();

private:
	HazeCompiler* Compiler;
	HAZE_STRING Name;
	std::vector<std::unique_ptr<ASTFunctionDefine>> Vector_FunctionExpression;
	std::vector<std::unique_ptr<ASTClassDefine>> Vector_ClassExpression;
};
