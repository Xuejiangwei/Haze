#pragma once

#include "ASTBase.h"

class ASTFunctionDefine;

class ASTStandardLibrary
{
public:
	ASTStandardLibrary(HazeVM* VM, HAZE_STRING& Name, std::vector<std::unique_ptr<ASTFunctionDefine>>& Vector_Expression);
	~ASTStandardLibrary();

	void CodeGen();

private:
	HazeVM* VM;
	HAZE_STRING Name;
	std::vector<std::unique_ptr<ASTFunctionDefine>> Vector_Expression;
};
