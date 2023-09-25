#include "HazeCompiler.h"
#include "HazeCompilerModule.h"

#include "ASTFunction.h"
#include "ASTClass.h"
#include "ASTLibrary.h"

ASTLibrary::ASTLibrary(HazeCompiler* m_Compiler, /*const SourceLocation& Location,*/ HAZE_STRING& m_Name, HazeLibraryType Type,
	std::vector<std::unique_ptr<ASTFunctionDefine>>& Vector_FunctionExpression, std::vector<std::unique_ptr<ASTClassDefine>>& Vector_ClassExpression) :
	m_Compiler(m_Compiler), m_Name(std::move(m_Name)), Type(Type), Vector_FunctionExpression(std::move(Vector_FunctionExpression)),
	Vector_ClassExpression(std::move(Vector_ClassExpression))
{
}

ASTLibrary::~ASTLibrary()
{
}

void ASTLibrary::CodeGen()
{
	m_Compiler->GetCurrModule()->MarkLibraryType(Type);
	for (auto& Iter : Vector_FunctionExpression)
	{
		Iter->CodeGen();
	}

	for (auto& Iter : Vector_ClassExpression)
	{
		Iter->CodeGen();
	}
}