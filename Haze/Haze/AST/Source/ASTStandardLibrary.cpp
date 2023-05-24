#include "HazeVM.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"

#include "ASTFunction.h"
#include "ASTClass.h"
#include "ASTStandardLibrary.h"

ASTStandardLibrary::ASTStandardLibrary(HazeVM* VM, HAZE_STRING& Name, std::vector<std::unique_ptr<ASTFunctionDefine>>& Vector_FunctionExpression,
	std::vector<std::unique_ptr<ASTClassDefine>>& Vector_ClassExpression) :
	VM(VM), Name(std::move(Name)), Vector_FunctionExpression(std::move(Vector_FunctionExpression)),
	Vector_ClassExpression(std::move(Vector_ClassExpression))
{
}

ASTStandardLibrary::~ASTStandardLibrary()
{
}

void ASTStandardLibrary::CodeGen()
{
	VM->GetCompiler()->GetCurrModule()->MarkStandardLibrary();
	for (auto& Iter : Vector_FunctionExpression)
	{
		Iter->CodeGen();
	}

	for (auto& Iter : Vector_ClassExpression)
	{
		Iter->CodeGen();
	}
}
