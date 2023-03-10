#include "ASTFunction.h"
#include "HazeVM.h"

#include "HazeCompiler.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerModule.h"

ASTFunction::ASTFunction(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param, std::unique_ptr<ASTBase>& Body)
	: VM(VM), Section(Section),
	FunctionName(std::move(Name)),
	FunctionType(std::move(Type)),
	Vector_FunctionParam(std::move(Param)),
	Body(std::move(Body))
{
}

ASTFunction::~ASTFunction()
{
}

HazeValue* ASTFunction::CodeGen()
{
	auto& Compiler = VM->GetCompiler();
	auto& Module = Compiler->GetCurrModule();

	std::shared_ptr<HazeCompilerFunction> CompilerFunction = nullptr;

	if (Section == HazeSectionSignal::Global)
	{
		CompilerFunction = Module->CreateFunction(FunctionName, FunctionType, Vector_FunctionParam);

		std::shared_ptr<HazeBaseBlock> BB = HazeBaseBlock::CreateBaseBlock(HAZE_TEXT("entry"), CompilerFunction.get());
		Compiler->SetInsertBlock(BB);
	
		for (auto& iter : Vector_FunctionParam)
		{
			Compiler->CreateLocalVariable(CompilerFunction, iter);
		}
	}

	if (Body)
	{
		Body->CodeGen();
	}
	

	//���ɺ����ֽ���

	if (CompilerFunction)
	{
		CompilerFunction->FunctionFinish();
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

ASTFunctionDefine::ASTFunctionDefine(HazeVM* VM, const HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param) 
	: VM(VM), FunctionName(Name), FunctionType(Type), Vector_FunctionParam(std::move(Param))
{
}

ASTFunctionDefine::~ASTFunctionDefine()
{
}

void ASTFunctionDefine::CodeGen()
{
	auto& Compiler = VM->GetCompiler();
	auto& Module = Compiler->GetCurrModule();
	std::shared_ptr<HazeCompilerFunction> CompilerFunction = Module->CreateFunction(FunctionName, FunctionType, Vector_FunctionParam);

	std::shared_ptr<HazeBaseBlock> BB = HazeBaseBlock::CreateBaseBlock(HAZE_TEXT("entry"), CompilerFunction.get());
	Compiler->SetInsertBlock(BB);

	for (auto& iter : Vector_FunctionParam)
	{
		Compiler->CreateLocalVariable(CompilerFunction, iter);
	}
}
