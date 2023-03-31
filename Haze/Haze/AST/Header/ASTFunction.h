#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTBase.h"

class ASTFunction
{
public:
	ASTFunction(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param, std::unique_ptr<ASTBase>& Body);

	~ASTFunction();

	HazeValue* CodeGen();

private:
	HazeVM* VM;
	HazeSectionSignal Section;

	HAZE_STRING FunctionName;
	HazeDefineType FunctionType;
	std::vector<HazeDefineVariable> Vector_FunctionParam; //������
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

class ASTClassFunctionSection
{
public:
	ASTClassFunctionSection(HazeVM* VM, std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>>& Functions);
	~ASTClassFunctionSection();

	void CodeGen();
private:
	HazeVM* VM;
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>> Functions;
};

class ASTFunctionDefine
{
public:
	ASTFunctionDefine(HazeVM* VM, const HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param);
	~ASTFunctionDefine();

	void CodeGen();

private:
	HazeVM* VM;
	HAZE_STRING FunctionName;
	HazeDefineType FunctionType;
	std::vector<HazeDefineVariable> Vector_FunctionParam; //������
};
