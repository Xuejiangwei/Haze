#pragma once

#include "ASTBase.h"

class ASTFunctionDefine;
class ASTClassDefine;

class ASTLibrary
{
public:
	ASTLibrary(HazeCompiler* compiler, /*const SourceLocation& Location,*/ HString& name, HazeLibraryType type,
		V_Array<Unique<ASTFunctionDefine>>& functionExpressions,
		V_Array<Unique<ASTClassDefine>>& classExpressions);

	~ASTLibrary();

	void CodeGen();

private:
	HazeCompiler* m_Compiler;
	HString m_Name;
	HazeLibraryType m_Type;
	V_Array<Unique<ASTFunctionDefine>> m_FunctionExpressions;
	V_Array<Unique<ASTClassDefine>> m_ClassExpressions;
};