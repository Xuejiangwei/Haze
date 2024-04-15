#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTBase.h"

class ASTFunction
{
public:
	ASTFunction(HazeCompiler* compiler, const SourceLocation& startLocation, const SourceLocation& endLocation, HazeSectionSignal section,
		HAZE_STRING& name, HazeDefineType& type, std::vector<std::unique_ptr<ASTBase>>& params, std::unique_ptr<ASTBase>& body);

	~ASTFunction();

	HazeValue* CodeGen();

	void RegisterFunction();

	const HAZE_STRING& GetName() const { return m_FunctionName; }

private:
	HazeCompiler* m_Compiler;
	HazeSectionSignal m_Section;

	HAZE_STRING m_FunctionName;
	HazeDefineType m_FunctionType;
	std::vector<std::unique_ptr<ASTBase>> m_FunctionParams; //从左到右
	std::unique_ptr<ASTBase> m_Body;

	SourceLocation m_StartLocation;
	SourceLocation m_EndLocation;
};

class ASTFunctionSection
{
public:
	ASTFunctionSection(HazeCompiler* compiler,/* const SourceLocation& Location,*/ std::vector<std::unique_ptr<ASTFunction>>& functions);

	~ASTFunctionSection();

	void CodeGen();

private:
	HazeCompiler* m_Compiler;
	std::vector<std::unique_ptr<ASTFunction>> m_Functions;
};

class ASTClassFunctionSection
{
public:
	ASTClassFunctionSection(HazeCompiler* compiler, /*const SourceLocation& Location,*/
		std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>>& functions);

	~ASTClassFunctionSection();

	void CodeGen();
private:
	HazeCompiler* m_Compiler;
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>> m_Functions;
};

class ASTFunctionDefine
{
public:
	ASTFunctionDefine(HazeCompiler* compiler,/* const SourceLocation& Location,*/ const HAZE_STRING& name, HazeDefineType& type,
		std::vector<std::unique_ptr<ASTBase>>& params);

	~ASTFunctionDefine();

	void CodeGen();

private:
	HazeCompiler* m_Compiler;
	HAZE_STRING m_FunctionName;
	HazeDefineType m_FunctionType;
	std::vector<std::unique_ptr<ASTBase>> m_FunctionParams; //从左到右
};
