#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTBase.h"

class ASTFunction
{
public:
	ASTFunction(HazeCompiler* m_Compiler, const SourceLocation& StartLocation, const SourceLocation& EndLocation, HazeSectionSignal Section, HAZE_STRING& m_Name, HazeDefineType& Type,
		std::vector<std::unique_ptr<ASTBase>>& Param, std::unique_ptr<ASTBase>& Body);

	~ASTFunction();

	HazeValue* CodeGen();

	void RegisterFunction();

private:
	HazeCompiler* m_Compiler;
	HazeSectionSignal Section;

	HAZE_STRING FunctionName;
	HazeDefineType FunctionType;
	std::vector<std::unique_ptr<ASTBase>> Vector_FunctionParam; //������
	std::unique_ptr<ASTBase> Body;

	SourceLocation StartLocation;
	SourceLocation EndLocation;
};

class ASTFunctionSection
{
public:
	ASTFunctionSection(HazeCompiler* m_Compiler,/* const SourceLocation& Location,*/ std::vector<std::unique_ptr<ASTFunction>>& Functions);
	~ASTFunctionSection();

	void CodeGen();

private:
	HazeCompiler* m_Compiler;
	std::vector<std::unique_ptr<ASTFunction>> Functions;
};

class ASTClassFunctionSection
{
public:
	ASTClassFunctionSection(HazeCompiler* m_Compiler, /*const SourceLocation& Location,*/ std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>>& Functions);
	~ASTClassFunctionSection();

	void CodeGen();
private:
	HazeCompiler* m_Compiler;
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>> Functions;
};

class ASTFunctionDefine
{
public:
	ASTFunctionDefine(HazeCompiler* m_Compiler,/* const SourceLocation& Location,*/ const HAZE_STRING& m_Name, HazeDefineType& Type, std::vector<std::unique_ptr<ASTBase>>& Param);
	~ASTFunctionDefine();

	void CodeGen();

private:
	HazeCompiler* m_Compiler;
	HAZE_STRING FunctionName;
	HazeDefineType FunctionType;
	std::vector<std::unique_ptr<ASTBase>> Vector_FunctionParam; //������
};
