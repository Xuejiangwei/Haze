#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTBase.h"

class ASTFunction
{
public:
	ASTFunction(HazeCompiler* compiler, const SourceLocation& startLocation, const SourceLocation& endLocation, HazeSectionSignal section,
		HString& name, HazeDefineType& type, V_Array<Unique<ASTBase>>& params, Unique<ASTBase>& body);

	~ASTFunction();

	HazeValue* CodeGen();

	void RegisterFunction();

	const HString& GetName() const { return m_FunctionName; }

private:
	HazeCompiler* m_Compiler;
	HazeSectionSignal m_Section;

	HString m_FunctionName;
	HazeDefineType m_FunctionType;
	V_Array<Unique<ASTBase>> m_FunctionParams; //������
	Unique<ASTBase> m_Body;

	SourceLocation m_StartLocation;
	SourceLocation m_EndLocation;
};

class ASTFunctionSection
{
public:
	ASTFunctionSection(HazeCompiler* compiler,/* const SourceLocation& Location,*/ V_Array<Unique<ASTFunction>>& functions);

	~ASTFunctionSection();

	void CodeGen();

private:
	HazeCompiler* m_Compiler;
	V_Array<Unique<ASTFunction>> m_Functions;
};

class ASTClassFunctionSection
{
public:
	ASTClassFunctionSection(HazeCompiler* compiler, /*const SourceLocation& Location,*/
		V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>>& functions);

	~ASTClassFunctionSection();

	void CodeGen();
private:
	HazeCompiler* m_Compiler;
	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>> m_Functions;
};

class ASTFunctionDefine
{
public:
	ASTFunctionDefine(HazeCompiler* compiler,/* const SourceLocation& Location,*/ const HString& name, HazeDefineType& type,
		V_Array<Unique<ASTBase>>& params);

	~ASTFunctionDefine();

	void CodeGen();

private:
	HazeCompiler* m_Compiler;
	HString m_FunctionName;
	HazeDefineType m_FunctionType;
	V_Array<Unique<ASTBase>> m_FunctionParams; //������
};
