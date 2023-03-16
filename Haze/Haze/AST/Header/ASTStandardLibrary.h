#pragma once

#include "ASTBase.h"

class ASTFunctionDefine;
class ASTClassDefine;

class ASTStandardLibrary
{
public:
	ASTStandardLibrary(HazeVM* VM, HAZE_STRING& Name, std::vector<std::unique_ptr<ASTFunctionDefine>>& Vector_FunctionExpression, 
		std::vector<std::unique_ptr<ASTClassDefine>>& Vector_ClassExpression);
	~ASTStandardLibrary();

	void CodeGen();

private:
	HazeVM* VM;
	HAZE_STRING Name;
	std::vector<std::unique_ptr<ASTFunctionDefine>> Vector_FunctionExpression;
	std::vector<std::unique_ptr<ASTClassDefine>> Vector_ClassExpression;
};
