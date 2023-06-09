#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTBase.h"

class ASTFunction
{
public:
	ASTFunction(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param, std::unique_ptr<ASTBase>& Body);

	~ASTFunction();

	HazeValue* CodeGen();

	void RegisterFunction();

private:
	HazeCompiler* Compiler;
	HazeSectionSignal Section;

	HAZE_STRING FunctionName;
	HazeDefineType FunctionType;
	std::vector<HazeDefineVariable> Vector_FunctionParam; //从左到右
	std::unique_ptr<ASTBase> Body;

	SourceLocation Location;
};

class ASTFunctionSection
{
public:
	ASTFunctionSection(HazeCompiler* Compiler,/* const SourceLocation& Location,*/ std::vector<std::unique_ptr<ASTFunction>>& Functions);
	~ASTFunctionSection();

	void CodeGen();

private:
	HazeCompiler* Compiler;
	std::vector<std::unique_ptr<ASTFunction>> Functions;
};

class ASTClassFunctionSection
{
public:
	ASTClassFunctionSection(HazeCompiler* Compiler, /*const SourceLocation& Location,*/ std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>>& Functions);
	~ASTClassFunctionSection();

	void CodeGen();
private:
	HazeCompiler* Compiler;
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>> Functions;
};

class ASTFunctionDefine
{
public:
	ASTFunctionDefine(HazeCompiler* Compiler,/* const SourceLocation& Location,*/ const HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param);
	~ASTFunctionDefine();

	void CodeGen();

private:
	HazeCompiler* Compiler;
	HAZE_STRING FunctionName;
	HazeDefineType FunctionType;
	std::vector<HazeDefineVariable> Vector_FunctionParam; //从左到右
};
