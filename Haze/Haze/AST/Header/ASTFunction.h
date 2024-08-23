#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTBase.h"

class ASTFunction
{
public:
	ASTFunction(Compiler* compiler, const SourceLocation& startLocation, const SourceLocation& endLocation, HazeSectionSignal section,
		HString& name, HazeDefineType& type, V_Array<Unique<ASTBase>>& params, Unique<ASTBase>& body);

	~ASTFunction();

	HazeValue* CodeGen();

	void RegisterFunction();

	const HString& GetName() const { return m_FunctionName; }

private:
	Compiler* m_Compiler;
	HazeSectionSignal m_Section;

	HString m_FunctionName;
	HazeDefineType m_FunctionType;
	V_Array<Unique<ASTBase>> m_FunctionParams; //从左到右
	Unique<ASTBase> m_Body;

	SourceLocation m_StartLocation;
	SourceLocation m_EndLocation;
};

class ASTFunctionSection
{
public:
	ASTFunctionSection(Compiler* compiler,/* const SourceLocation& Location,*/ V_Array<Unique<ASTFunction>>& functions);

	~ASTFunctionSection();

	void CodeGen();

private:
	Compiler* m_Compiler;
	V_Array<Unique<ASTFunction>> m_Functions;
};

class ASTClassFunctionSection
{
public:
	ASTClassFunctionSection(Compiler* compiler, /*const SourceLocation& Location,*/
		V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>>& functions);

	~ASTClassFunctionSection();

	void CodeGen();
private:
	Compiler* m_Compiler;
	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>> m_Functions;
};

class ASTFunctionDefine
{
public:
	ASTFunctionDefine(Compiler* compiler,/* const SourceLocation& Location,*/ const HString& name, HazeDefineType& type,
		V_Array<Unique<ASTBase>>& params);

	~ASTFunctionDefine();

	void CodeGen();

private:
	Compiler* m_Compiler;
	HString m_FunctionName;
	HazeDefineType m_FunctionType;
	V_Array<Unique<ASTBase>> m_FunctionParams; //从左到右
};
