#include "ASTBase.h"
#include "ASTFunction.h"
#include "ASTClass.h"

ASTClass::ASTClass(HazeVM* VM, HAZE_STRING& Name, std::vector<std::vector<std::unique_ptr<ASTBase>>>& Data, std::unique_ptr<ASTFunctionSection>& FunctionSection)
	: VM(VM), ClassName(std::move(Name)), Vector_ClassData(std::move(Data)), ClassFunctionSection(std::move(FunctionSection))
{
}

ASTClass::~ASTClass()
{
}

void ASTClass::CodeGen()
{

}

ASTClassDefine::ASTClassDefine(HazeVM* VM, HAZE_STRING& Name, std::vector<std::vector<std::unique_ptr<ASTBase>>>& Data, std::vector<std::unique_ptr<ASTFunctionDefine>>& Function)
	: VM(VM), ClassName(std::move(Name)), ClassData(std::move(Data)), ClassFunction(std::move(Function))
{
}

ASTClassDefine::~ASTClassDefine()
{
}

void ASTClassDefine::CodeGen()
{
}
