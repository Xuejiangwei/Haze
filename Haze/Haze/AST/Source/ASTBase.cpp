#include "ASTBase.h"
#include "HazeVM.h"
#include "HazeCompiler.h"
#include "HazeCompilerValue.h"

//Base
ASTBase::ASTBase(HazeVM* VM) : VM(VM)
{
	Value.Type = HazeValueType::Int;
	Value.Value.IntValue = 0;
}

ASTBase::ASTBase(HazeVM* VM, HazeValue Value) : VM(VM), Value(Value)
{
}

ASTBase::~ASTBase()
{
}

ASTBool::ASTBool(HazeVM* VM, HazeValue V) : ASTBase(VM, V)
{
}

ASTBool::~ASTBool()
{
}

HazeCompilerValue* ASTBool::CodeGen()
{
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	HazeCompilerValue* RetValue = Compiler->GenDataVariable(Value);

	return RetValue;
}

ASTNumber::ASTNumber(HazeVM* VM, HazeValue V) : ASTBase(VM, V)
{
}

ASTNumber::~ASTNumber()
{
}

HazeCompilerValue* ASTNumber::CodeGen()
{
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	HazeCompilerValue* RetValue = Compiler->GenDataVariable(Value);

	return RetValue;
}

ASTVariableDefine::ASTVariableDefine(HazeVM* VM, HazeSectionSignal Section, HazeValueType Type, HAZE_STRING& Name, std::unique_ptr<ASTBase>& Expression)
	: ASTBase(VM), SectionSignal(Section), Type(Type), Name(std::move(Name)), Expression(std::move(Expression))
{
}

ASTVariableDefine::~ASTVariableDefine()
{
}

HazeCompilerValue* ASTVariableDefine::CodeGen()
{
	HazeCompilerValue* RetValue = nullptr;
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	if (SectionSignal == HazeSectionSignal::Global)
	{
		//生成全局变量字节码
		RetValue = Compiler->GenGlobalVariable(Name, Type);
	}
	else if (SectionSignal == HazeSectionSignal::Local)
	{
		//生成局部变量字节码
		RetValue = Compiler->GenLocalVariable();
	}

	HazeCompilerValue* ExprValue = nullptr;
	if (Expression)
	{
		ExprValue = Expression->CodeGen();

		RetValue->StoreValue(ExprValue);
	}

	return RetValue;
}