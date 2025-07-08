#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTBase.h"

class ASTFunction
{
public:
	ASTFunction(Compiler* compiler, const SourceLocation& startLocation, const SourceLocation& endLocation, HazeSectionSignal section,
		HString& name, HazeVariableType& type, V_Array<Unique<ASTBase>>& params, Unique<ASTBase> body, bool isVirtual, bool isPureVirtual);

	~ASTFunction();

	HazeValue* CodeGen();

	void RegisterFunction();

	const HString& GetName() const { return m_FunctionName; }

private:
	Compiler* m_Compiler;
	HazeSectionSignal m_Section;
	bool m_IsVirtual;
	bool m_IsPureVirtual;

	HString m_FunctionName;
	HazeVariableType m_FunctionType;
	V_Array<Unique<ASTBase>> m_FunctionParams; //������
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
	ASTFunctionDefine(Compiler* compiler,/* const SourceLocation& Location,*/ const HString& name, HazeVariableType& type,
		V_Array<Unique<ASTBase>>& params);

	~ASTFunctionDefine();

	void CodeGen();

private:
	Compiler* m_Compiler;
	HString m_FunctionName;
	HazeVariableType m_FunctionType;
	V_Array<Unique<ASTBase>> m_FunctionParams; //������
};
