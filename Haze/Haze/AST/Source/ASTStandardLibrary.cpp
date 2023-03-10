#include "HazeVM.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"

#include "ASTFunction.h"
#include "ASTStandardLibrary.h"

ASTStandardLibrary::ASTStandardLibrary(HazeVM* VM, HAZE_STRING& Name, std::vector<std::unique_ptr<ASTFunctionDefine>>& Vector_Expression)
	: VM(VM), Name(std::move(Name)), Vector_Expression(std::move(Vector_Expression))
{
}

ASTStandardLibrary::~ASTStandardLibrary()
{
}

void ASTStandardLibrary::CodeGen()
{
	VM->GetCompiler()->GetCurrModule()->MarkStandardLibrary();
	for (auto& Iter : Vector_Expression)
	{
		Iter->CodeGen();
	}
}
