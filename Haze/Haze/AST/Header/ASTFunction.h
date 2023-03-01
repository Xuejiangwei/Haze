#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTBase.h"

class ASTFunction
{
public:
	ASTFunction(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param, std::unique_ptr<ASTBase>& Body);

	~ASTFunction();

	HazeValue* CodeGen();

private:
	HazeVM* VM;
	HazeSectionSignal Section;

	HAZE_STRING FunctionName;
	HazeDefineData FunctionType;
	std::vector<HazeDefineVariable> FunctionParam; //´Ó×óµ½ÓÒ
	std::unique_ptr<ASTBase> Body;
};


class ASTFunctionSection
{
public:
	ASTFunctionSection(HazeVM* VM, std::vector<std::unique_ptr<ASTFunction>>& Functions);
	~ASTFunctionSection();

	void CodeGen();

private:
	HazeVM* VM;
	std::vector<std::unique_ptr<ASTFunction>> Functions;
};

