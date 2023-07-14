#include "ASTFunction.h"

#include "HazeCompiler.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerModule.h"

ASTFunction::ASTFunction(HazeCompiler* Compiler, const SourceLocation& StartLocation, const SourceLocation& EndLocation, HazeSectionSignal Section, HAZE_STRING& Name, 
	HazeDefineType& Type, std::vector<std::unique_ptr<ASTBase>>& Param, std::unique_ptr<ASTBase>& Body) 
	: Compiler(Compiler), StartLocation(StartLocation), EndLocation(EndLocation), Section(Section), FunctionName(std::move(Name)),FunctionType(std::move(Type)),
	Vector_FunctionParam(std::move(Param)), Body(std::move(Body))
{
}

ASTFunction::~ASTFunction()
{
}

HazeValue* ASTFunction::CodeGen()
{
	auto& Module = Compiler->GetCurrModule();

	std::shared_ptr<HazeCompilerFunction> CompilerFunction = nullptr;
	std::shared_ptr<HazeCompilerClass> Class = nullptr;

	std::vector<HazeDefineVariable> Vector_ParamDefine(Vector_FunctionParam.size());
	for (size_t i = 0; i < Vector_FunctionParam.size(); i++)
	{
		Vector_ParamDefine[i] = Vector_FunctionParam[i]->GetDefine();
	}

	if (Section == HazeSectionSignal::Global)
	{
		CompilerFunction = Module->CreateFunction(FunctionName, FunctionType, Vector_ParamDefine);
	}
	else if (Section == HazeSectionSignal::Class)
	{
		Class = Module->GetClass(Vector_FunctionParam[0]->GetDefine().Type.CustomName);
		CompilerFunction = Module->CreateFunction(Class, FunctionName, FunctionType, Vector_ParamDefine);
	}

	Compiler->SetInsertBlock(CompilerFunction->GetEntryBlock());

	for (int i = (int)Vector_FunctionParam.size() - 1; i >= 0; i--)
	{
		Vector_FunctionParam[i]->CodeGen();
	}

	if (Body)
	{
		Body->CodeGen();
	}

	if (FunctionType.PrimaryType == HazeValueType::Void)
	{
		Compiler->InsertLineCount(StartLocation.Line);
	}

	if (CompilerFunction)
	{
		CompilerFunction->FunctionFinish();
	}

	return nullptr;
}

void ASTFunction::RegisterFunction()
{
	auto& Module = Compiler->GetCurrModule();

	std::shared_ptr<HazeCompilerClass> Class = nullptr;

	std::vector<HazeDefineVariable> Vector_ParamDefine(Vector_FunctionParam.size());
	for (size_t i = 0; i < Vector_FunctionParam.size(); i++)
	{
		Vector_ParamDefine[i] = Vector_FunctionParam[i]->GetDefine();
	}

	if (Section == HazeSectionSignal::Global)
	{
		Module->CreateFunction(FunctionName, FunctionType, Vector_ParamDefine);
	}
	else if (Section == HazeSectionSignal::Class)
	{
		Class = Module->GetClass(Vector_FunctionParam[0]->GetDefine().Type.CustomName);
		Module->CreateFunction(Class, FunctionName, FunctionType, Vector_ParamDefine);
	}
}

ASTFunctionSection::ASTFunctionSection(HazeCompiler* Compiler,/* const SourceLocation& Location,*/ std::vector<std::unique_ptr<ASTFunction>>& Functions)
	: Compiler(Compiler), Functions(std::move(Functions))
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

ASTFunctionDefine::ASTFunctionDefine(HazeCompiler* Compiler, /*const SourceLocation& Location,*/ const HAZE_STRING& Name, HazeDefineType& Type, 
	std::vector<std::unique_ptr<ASTBase>>& Param)
	: Compiler(Compiler), FunctionName(Name), FunctionType(Type), Vector_FunctionParam(std::move(Param))
{
}

ASTFunctionDefine::~ASTFunctionDefine()
{
}

void ASTFunctionDefine::CodeGen()
{
	auto& Module = Compiler->GetCurrModule();

	std::vector<HazeDefineVariable> Vector_ParamDefine(Vector_FunctionParam.size());
	for (size_t i = 0; i < Vector_FunctionParam.size(); i++)
	{
		Vector_ParamDefine[i] = Vector_FunctionParam[i]->GetDefine();
	}

	std::shared_ptr<HazeCompilerFunction> CompilerFunction = Module->CreateFunction(FunctionName, FunctionType, Vector_ParamDefine);

	Compiler->SetInsertBlock(CompilerFunction->GetEntryBlock());

	for (auto& Iter : Vector_FunctionParam)
	{
		Iter->CodeGen();
	}
}

ASTClassFunctionSection::ASTClassFunctionSection(HazeCompiler* Compiler, /*const SourceLocation& Location,*/ std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>>& Functions)
	: Compiler(Compiler), Functions(std::move(Functions))
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