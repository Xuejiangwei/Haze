#include "HazePch.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"

#include "ASTFunction.h"
#include "ASTClass.h"
#include "ASTLibrary.h"

ASTLibrary::ASTLibrary(HazeCompiler* compiler, /*const SourceLocation& Location,*/ HString& name, HazeLibraryType type,
	V_Array<Unique<ASTFunctionDefine>>& functionExpressions, V_Array<Unique<ASTClassDefine>>& classExpressions) :
	m_Compiler(compiler), m_Name(Move(name)), m_Type(type), m_FunctionExpressions(Move(functionExpressions)),
	m_ClassExpressions(Move(classExpressions))
{
}

ASTLibrary::~ASTLibrary()
{
}

void ASTLibrary::CodeGen()
{
	m_Compiler->GetCurrModule()->MarkLibraryType(m_Type);
	for (auto& iter : m_FunctionExpressions)
	{
		iter->CodeGen();
	}

	for (auto& iter : m_ClassExpressions)
	{
		iter->CodeGen();
	}
}