#include "ASTFunction.h"
#include "HazeVM.h"

#include "HazeCompiler.h"
#include "HazeCompilerModule.h"

ASTFunction::ASTFunction(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param, std::unique_ptr<ASTBase>& Body)
	: VM(VM), Section(Section),
	FunctionName(std::move(Name)),
	FunctionType(std::move(Type)),
	FunctionParam(std::move(Param)),
	Body(std::move(Body))
{
}

ASTFunction::~ASTFunction()
{
}

HazeValue* ASTFunction::CodeGen()
{
	auto& Module = VM->GetCompiler()->GetCurrModule();

	if (Section == HazeSectionSignal::Global)
	{
		Module->AddFunction(FunctionName, FunctionType, FunctionParam);
	}

	return nullptr;
}

ASTFunctionSection::ASTFunctionSection(HazeVM* VM, std::vector<std::unique_ptr<ASTFunction>>& Functions)
	: VM(VM), Functions(std::move(Functions))
{
}

ASTFunctionSection::~ASTFunctionSection()
{
}

void ASTFunctionSection::CodeGen()
{
	for (auto& it : Functions)
	{
		it->CodeGen();
	}
}