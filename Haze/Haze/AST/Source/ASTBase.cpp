#include "ASTBase.h"
#include "HazeVM.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerValue.h"

//Base
ASTBase::ASTBase(HazeVM* VM) : VM(VM)
{

}

ASTBase::ASTBase(HazeVM* VM, const HazeDefineVariable& Var) : VM(VM), DefineVariable(Var)
{
}

ASTBase::ASTBase(HazeVM* VM, const HazeValue& Value) : VM(VM), Value(Value)
{
}

ASTBase::~ASTBase()
{
}

ASTBool::ASTBool(HazeVM* VM, const HazeValue& Value) : ASTBase(VM, Value)
{
}

ASTBool::~ASTBool()
{
}

std::shared_ptr<HazeCompilerValue> ASTBool::CodeGen()
{
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	std::shared_ptr<HazeCompilerValue> RetValue = Compiler->GenConstantValue(Value);

	return RetValue;
}

ASTNumber::ASTNumber(HazeVM* VM, const HazeValue& Value) : ASTBase(VM, Value)
{
}

ASTNumber::~ASTNumber()
{
}

std::shared_ptr<HazeCompilerValue> ASTNumber::CodeGen()
{
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	std::shared_ptr<HazeCompilerValue> RetValue = Compiler->GenConstantValue(Value);

	return RetValue;
}

ASTIdentifier::ASTIdentifier(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name)
	: ASTBase(VM), SectionSignal(Section), Name(std::move(Name))
{
}

ASTIdentifier::~ASTIdentifier()
{
}

std::shared_ptr<HazeCompilerValue> ASTIdentifier::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> Value = nullptr;
	if (SectionSignal == HazeSectionSignal::Global)
	{
	}
	else if (SectionSignal == HazeSectionSignal::Function)
	{
		Value = VM->GetCompiler()->GetLocalVariable(Name);
		if (!Value)
		{
			Value = VM->GetCompiler()->GetGlobalVariable(Name);
		}
	}

	return Value;
}

ASTFunctionCall::ASTFunctionCall(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, std::vector<std::unique_ptr<ASTBase>>& FunctionParam)
	: ASTBase(VM), SectionSignal(Section), Name(std::move(Name)), FunctionParam(std::move(FunctionParam))
{
}

ASTFunctionCall::~ASTFunctionCall()
{
}

std::shared_ptr<HazeCompilerValue> ASTFunctionCall::CodeGen()
{
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	auto Function = Compiler->GetCurrModule()->GetFunction(Name);
	if (Function)
	{
		std::vector<std::shared_ptr<HazeCompilerValue>> Param;
		for (auto& it : FunctionParam)
		{
			Param.push_back(it->CodeGen());
		}

		return Compiler->CreateFunctionCall(Function, Param);
	}

	return nullptr;
}

ASTVariableDefine::ASTVariableDefine(HazeVM* VM, HazeSectionSignal Section, const HazeDefineVariable& DefineVariable, std::unique_ptr<ASTBase> Expression)
	: ASTBase(VM, DefineVariable), SectionSignal(Section), Expression(std::move(Expression))
{
}

ASTVariableDefine::~ASTVariableDefine()
{
}

std::shared_ptr<HazeCompilerValue> ASTVariableDefine::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> RetValue = nullptr;
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	std::unique_ptr<HazeCompilerModule>& Module = VM->GetCompiler()->GetCurrModule();
	if (SectionSignal == HazeSectionSignal::Global)
	{
		//生成全局变量字节码
		RetValue = Compiler->CreateGlobalVariable(Module, DefineVariable);
	}
	else if (SectionSignal == HazeSectionSignal::Function)
	{
		//生成局部变量字节码
		RetValue = Compiler->CreateLocalVariable(Module->GetCurrFunction(), DefineVariable);
	}

	std::shared_ptr<HazeCompilerValue> ExprValue = nullptr;
	if (Expression)
	{
		ExprValue = Expression->CodeGen();

		RetValue->StoreValue(ExprValue);
	}

	return RetValue;
}

ASTReturn::ASTReturn(HazeVM* VM, std::unique_ptr<ASTBase>& Expression)
	:ASTBase(VM), Expression(std::move(Expression))
{
}

ASTReturn::~ASTReturn()
{
}

std::shared_ptr<HazeCompilerValue> ASTReturn::CodeGen()
{
	return Expression->CodeGen();
}

ASTMutiExpression::ASTMutiExpression(HazeVM* VM, HazeSectionSignal Section, std::vector<std::unique_ptr<ASTBase>>& VectorExpression)
	: ASTBase(VM), SectionSignal(Section), VectorExpression(std::move(VectorExpression))
{
}

ASTMutiExpression::~ASTMutiExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTMutiExpression::CodeGen()
{
	for (auto& it : VectorExpression)
	{
		it->CodeGen();
	}
	return nullptr;
}

ASTBinaryExpression::ASTBinaryExpression(HazeVM* VM, HazeSectionSignal Section, HazeToken OperatorToken, std::unique_ptr<ASTBase>& LeftAST, std::unique_ptr<ASTBase>& RightAST)
	:ASTBase(VM), SectionSignal(Section), OperatorToken(OperatorToken), LeftAST(std::move(LeftAST)), RightAST(std::move(RightAST))
{
}

ASTBinaryExpression::~ASTBinaryExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTBinaryExpression::CodeGen()
{
	
	std::shared_ptr<HazeCompilerValue> LeftValue = LeftAST->CodeGen();
	std::shared_ptr<HazeCompilerValue> RightValue = RightAST->CodeGen();

	auto& Compiler = VM->GetCompiler();

	switch (OperatorToken)
	{
	case HazeToken::Add:
		return Compiler->CreateAdd(LeftValue, RightValue);
		break;
	case HazeToken::Sub:
		break;
	case HazeToken::Mul:
		break;
	case HazeToken::Div:
		break;
	case HazeToken::Mod:
		break;
	case HazeToken::And:
		break;
	case HazeToken::Or:
		break;
	case HazeToken::Not:
		break;
	case HazeToken::LeftMove:
		break;
	case HazeToken::RightMove:
		break;
	case HazeToken::Equal:
		break;
	case HazeToken::NotEqual:
		break;
	case HazeToken::Greater:
		break;
	case HazeToken::GreaterEqual:
		break;
	case HazeToken::Less:
		break;
	case HazeToken::LessEqual:
		break;
	case HazeToken::LeftParentheses:
		break;
	case HazeToken::RightParentheses:
		break;
	
	default:
		break;
	}

	return nullptr;
}


