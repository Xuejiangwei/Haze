#include "ASTBase.h"
#include "ASTFunction.h"
#include "ASTClass.h"

#include "HazeVM.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"

ASTClass::ASTClass(HazeVM* VM, HAZE_STRING& Name, std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTVariableDefine>>>>& Data, 
	std::unique_ptr<ASTClassFunctionSection>& FunctionSection)
	: VM(VM), ClassName(std::move(Name)), Vector_ClassData(std::move(Data)), ClassFunctionSection(std::move(FunctionSection))
{
}

ASTClass::~ASTClass()
{
}

void ASTClass::CodeGen()
{
	std::vector<std::pair<HazeDataDesc, std::vector<HazeDefineVariable*>>> Data;

	for (size_t i = 0; i < Vector_ClassData.size(); i++)
	{
		Data.push_back({ Vector_ClassData[i].first, {} });
		for (size_t j = 0; j < Vector_ClassData[i].second.size(); j++)
		{
			Data.back().second.push_back(&Vector_ClassData[i].second[j]->DefineVariable);
		}
	}

	VM->GetCompiler()->GetCurrModule()->CreateClass(ClassName, Data);

	if (ClassFunctionSection)
	{
		ClassFunctionSection->CodeGen();

	}
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
