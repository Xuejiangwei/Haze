#include "HazePch.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"

#include "ASTFunction.h"
#include "ASTClass.h"
#include "ASTLibrary.h"

ASTLibrary::ASTLibrary(HazeCompiler* compiler, /*const SourceLocation& Location,*/ HString& name, HazeLibraryType type,
	V_Array<Unique<ASTFunctionDefine>>& functionExpressions, V_Array<Unique<ASTClassDefine>>& classExpressions) :
	m_Compiler(compiler), m_Name(std::move(name)), m_Type(type), m_FunctionExpressions(std::move(functionExpressions)),
	m_ClassExpressions(std::move(classExpressions))
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