#include "HazeCompiler.h"
#include "HazeCompilerModule.h"

#include "ASTFunction.h"
#include "ASTClass.h"
#include "ASTLibrary.h"

ASTLibrary::ASTLibrary(HazeCompiler* compiler, /*const SourceLocation& Location,*/ HAZE_STRING& name, HazeLibraryType type,
	std::vector<std::unique_ptr<ASTFunctionDefine>>& functionExpressions, std::vector<std::unique_ptr<ASTClassDefine>>& classExpressions) :
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
	for (auto& Iter : m_FunctionExpressions)
	{
		Iter->CodeGen();
	}

	for (auto& Iter : m_ClassExpressions)
	{
		Iter->CodeGen();
	}
}