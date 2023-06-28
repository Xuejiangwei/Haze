#include "ASTBase.h"
#include "ASTFunction.h"
#include "ASTClass.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"

ASTClass::ASTClass(HazeCompiler* Compiler,/* const SourceLocation& Location,*/ HAZE_STRING& Name, std::vector<HAZE_STRING>& ParentClass,
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>>& Data, std::unique_ptr<ASTClassFunctionSection>& FunctionSection)
	: Compiler(Compiler), ClassName(std::move(Name)), ParentClass(std::move(ParentClass)), Vector_ClassData(std::move(Data)), ClassFunctionSection(std::move(FunctionSection))
{
}

ASTClass::~ASTClass()
{
}

void ASTClass::CodeGen()
{
	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>> Data;

	for (size_t i = 0; i < Vector_ClassData.size(); i++)
	{
		Data.push_back({ Vector_ClassData[i].first, {} });
		for (size_t j = 0; j < Vector_ClassData[i].second.size(); j++)
		{
			Data.back().second.push_back({ Vector_ClassData[i].second[j]->GetName(), Vector_ClassData[i].second[j]->CodeGen() });
		}
	}

	Compiler->GetCurrModule()->CreateClass(ClassName, Data);

	if (ClassFunctionSection)
	{
		ClassFunctionSection->CodeGen();
	}

	Compiler->GetCurrModule()->FinishCreateClass();
}

ASTClassDefine::ASTClassDefine(HazeCompiler* Compiler, /*const SourceLocation& Location,*/ HAZE_STRING& Name, std::vector<std::vector<std::unique_ptr<ASTBase>>>& Data, std::vector<std::unique_ptr<ASTFunctionDefine>>& Function)
	: Compiler(Compiler), ClassName(std::move(Name)), ClassData(std::move(Data)), ClassFunction(std::move(Function))
{
}

ASTClassDefine::~ASTClassDefine()
{
}

void ASTClassDefine::CodeGen()
{
}