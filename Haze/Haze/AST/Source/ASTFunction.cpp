#include "ASTFunction.h"
#include "HazeVM.h"

#include "HazeCompiler.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerModule.h"

ASTFunction::ASTFunction(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param, std::unique_ptr<ASTBase>& Body)
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
	std::shared_ptr<HazeCompilerClass> Class = nullptr;

	if (Section == HazeSectionSignal::Global)
	{
		CompilerFunction = Module->CreateFunction(FunctionName, FunctionType, Vector_FunctionParam);
	}
	else if (Section == HazeSectionSignal::Class)
	{
		Class = Module->FindClass(Vector_FunctionParam[0].Type.CustomName);
		CompilerFunction = Module->CreateFunction(Class, FunctionName, FunctionType, Vector_FunctionParam);
	}

	Compiler->SetInsertBlock(CompilerFunction->GetEntryBlock());

	for (int i = (int)Vector_FunctionParam.size() - 1; i >= 0; i--)
	{
		Compiler->CreateLocalVariable(CompilerFunction, Vector_FunctionParam[i]);
	}

	if (Body)
	{
		Body->CodeGen();
	}
	

	if (CompilerFunction)
	{
		CompilerFunction->FunctionFinish();
	}

	return nullptr;
}

void ASTFunction::RegisterFunction()
{
	auto& Compiler = VM->GetCompiler();
	auto& Module = Compiler->GetCurrModule();

	std::shared_ptr<HazeCompilerClass> Class = nullptr;

	if (Section == HazeSectionSignal::Global)
	{
		Module->CreateFunction(FunctionName, FunctionType, Vector_FunctionParam);
	}
	else if (Section == HazeSectionSignal::Class)
	{
		Class = Module->FindClass(Vector_FunctionParam[0].Type.CustomName);
		Module->CreateFunction(Class, FunctionName, FunctionType, Vector_FunctionParam);
	}
}

ASTFunctionSection::ASTFunctionSection(HazeVM* VM,std::vector<std::unique_ptr<ASTFunction>>& Functions)
	: VM(VM), Functions(std::move(Functions))
{
}

ASTFunctionSection::~ASTFunctionSection()
{
}

void ASTFunctionSection::CodeGen()
{
	for (auto& Iter : Functions)
	{
		Iter->RegisterFunction();
	}

	for (auto& Iter : Functions)
	{
		Iter->CodeGen();
	}
}

ASTFunctionDefine::ASTFunctionDefine(HazeVM* VM, const HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param) 
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

	Compiler->SetInsertBlock(CompilerFunction->GetEntryBlock());

	for (auto& Iter : Vector_FunctionParam)
	{
		Compiler->CreateLocalVariable(CompilerFunction, Iter);
	}
}

ASTClassFunctionSection::ASTClassFunctionSection(HazeVM* VM, std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>>& Functions)
	: VM(VM), Functions(std::move(Functions))
{

}

ASTClassFunctionSection::~ASTClassFunctionSection()
{
}

void ASTClassFunctionSection::CodeGen()
{
	for (auto& Iter : Functions)
	{
		for (auto& Function : Iter.second)
		{
			Function->RegisterFunction();
		}
	}

	for (auto& Iter : Functions)
	{
		for (auto& Function : Iter.second)
		{
			Function->CodeGen();
		}
	}
}
