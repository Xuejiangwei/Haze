#include "ASTBase.h"

#include "HazeVM.h"
#include "HazeCompilerHelper.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerHelper.h"

//Base
ASTBase::ASTBase(HazeVM* VM) : VM(VM)
{
	DefineVariable.Name = HAZE_TEXT("");
	DefineVariable.Type = { HazeValueType::Void, HAZE_TEXT("") };
}

ASTBase::ASTBase(HazeVM* VM, const HazeDefineVariable& Var) : VM(VM), DefineVariable(Var)
{
	Value.Type = HazeValueType::Void;
	memset(&Value.Value, 0, sizeof(Value.Value));
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

ASTStringText::ASTStringText(HazeVM* VM, HAZE_STRING& Text) : ASTBase(VM), Text(std::move(Text))
{
}

ASTStringText::~ASTStringText()
{
}

std::shared_ptr<HazeCompilerValue> ASTStringText::CodeGen()
{
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	std::shared_ptr<HazeCompilerValue> RetValue = Compiler->GenStringVariable(Text);

	return RetValue;
}


ASTIdentifier::ASTIdentifier(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HAZE_STRING* MemberName)
	: ASTBase(VM), SectionSignal(Section), Name(std::move(Name))
{
	if (MemberName)
	{
		ClassMemberName = std::move(*MemberName);
	}
}

ASTIdentifier::~ASTIdentifier()
{
}

std::shared_ptr<HazeCompilerValue> ASTIdentifier::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> RetValue = nullptr;
	if (SectionSignal == HazeSectionSignal::Global)
	{
	}
	else if (SectionSignal == HazeSectionSignal::Function)
	{
		RetValue = VM->GetCompiler()->GetLocalVariable(Name);
		if (!RetValue)
		{
			RetValue = VM->GetCompiler()->GetGlobalVariable(Name);
		}
	}

	return RetValue;
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
	auto Function = Compiler->GetFunction(Name);
	if (Function)
	{
		std::vector<std::shared_ptr<HazeCompilerValue>> Param;
		std::shared_ptr<HazeCompilerValue> ThisPointerValue = nullptr;
		if (Function->GetClass())
		{
			ThisPointerValue = Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(GetObjectName(Name));
		}

		for (auto& it : FunctionParam)
		{
			Param.push_back(it->CodeGen());
		}

		return Compiler->CreateFunctionCall(Function, Param, ThisPointerValue);
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
		//����ȫ�ֱ����ֽ���
		RetValue = Compiler->CreateGlobalVariable(Module, DefineVariable);
	}
	else if (SectionSignal == HazeSectionSignal::Function)
	{
		//���ɾֲ������ֽ���
		RetValue = Compiler->CreateLocalVariable(Module->GetCurrFunction(), DefineVariable);
	}

	std::shared_ptr<HazeCompilerValue> ExprValue = nullptr;
	if (Expression)
	{
		ExprValue = Expression->CodeGen();

		Compiler->CreateMov(RetValue, ExprValue);
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
	std::shared_ptr<HazeCompilerValue> RetValue = Expression->CodeGen();
	return VM->GetCompiler()->CreateRet(RetValue);
}

ASTNew::ASTNew(HazeVM* VM, const HazeDefineVariable& Define) : ASTBase(VM, Define)
{
}

ASTNew::~ASTNew()
{
}

std::shared_ptr<HazeCompilerValue> ASTNew::CodeGen()
{
	return VM->GetCompiler()->CreateNew(VM->GetCompiler()->GetCurrModule()->GetCurrFunction(), DefineVariable.Type);
}


ASTMultiExpression::ASTMultiExpression(HazeVM* VM, HazeSectionSignal Section, std::vector<std::unique_ptr<ASTBase>>& VectorExpression)
	: ASTBase(VM), SectionSignal(Section), VectorExpression(std::move(VectorExpression))
{
}

ASTMultiExpression::~ASTMultiExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTMultiExpression::CodeGen()
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
		return Compiler->CreateSub(LeftValue, RightValue);
		break;
	case HazeToken::Mul:
		return Compiler->CreateMul(LeftValue, RightValue);
		break;
	case HazeToken::Div:
		return Compiler->CreateDiv(LeftValue, RightValue);
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
	case HazeToken::Assign:
	{
		Compiler->CreateMov(LeftValue, RightValue);
		return LeftValue;
	}
		break;
	case HazeToken::Equal:
	case HazeToken::NotEqual:
	case HazeToken::Greater:
	case HazeToken::GreaterEqual:
	case HazeToken::Less:
	case HazeToken::LessEqual:
		return Compiler->CreateIntCmp(LeftValue, RightValue);
	case HazeToken::LeftParentheses:
		break;
	case HazeToken::RightParentheses:
		break;
	
	default:
		break;
	}

	return nullptr;
}

ASTImportModule::ASTImportModule(HazeVM* VM, const HAZE_STRING& ModuleName) : ASTBase(VM), ModuleName(ModuleName)
{
}

ASTImportModule::~ASTImportModule()
{
}

std::shared_ptr<HazeCompilerValue> ASTImportModule::CodeGen()
{
	VM->ParseModule(ModuleName);
	return nullptr;
}

ASTIfExpression::ASTIfExpression(HazeVM* VM, std::unique_ptr<ASTBase>& Condition, std::unique_ptr<ASTBase>& IfExpression, std::unique_ptr<ASTBase>& ElseExpression)
	: ASTBase(VM), Condition(std::move(Condition)), IfExpression(std::move(IfExpression)), ElseExpression(std::move(ElseExpression))
{
}

ASTIfExpression::~ASTIfExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTIfExpression::CodeGen()
{
	
	auto ConditionExp = static_cast<ASTBinaryExpression*>(Condition.get());

	auto& Compiler = VM->GetCompiler();
	auto Function = Compiler->GetCurrModule()->GetCurrFunction();

	Condition->CodeGen();

	auto ParentBlock = Compiler->GetInsertBlock();

	auto IfBlock = HazeBaseBlock::CreateBaseBlock(Function->GenIfBlockName(), Function, ParentBlock);
	IfExpression->CodeGen();
	IfBlock->FinishBlock();

	auto ElseBlock = HazeBaseBlock::CreateBaseBlock(Function->GenElseBlockName(), Function, ParentBlock);
	ElseExpression->CodeGen();
	ElseBlock->FinishBlock();

	Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(ConditionExp->OperatorToken), IfBlock, ElseBlock);


	return nullptr;
}

ASTWhileExpression::ASTWhileExpression(HazeVM* VM, std::unique_ptr<ASTBase>& Condition, std::unique_ptr<ASTBase>& MultiExpression)
	:ASTBase(VM), Condition(std::move(Condition)), MultiExpression(std::move(MultiExpression))
{
}

ASTWhileExpression::~ASTWhileExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTWhileExpression::CodeGen()
{
	auto& Compiler = VM->GetCompiler();
	auto Function = Compiler->GetCurrModule()->GetCurrFunction();
	auto ConditionExp = static_cast<ASTBinaryExpression*>(Condition.get());

	auto WhileBlockName = Function->GenWhileBlockName();

	auto ParentBlock = Compiler->GetInsertBlock();

	auto WhileBlock = HazeBaseBlock::CreateBaseBlock(WhileBlockName, Function, ParentBlock);
	ConditionExp->CodeGen();

	Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(ConditionExp->OperatorToken), nullptr, nullptr, false, true);

	MultiExpression->CodeGen();

	Compiler->CreateJmpToBlock(WhileBlock, true);

	WhileBlock->FinishBlock(nullptr, false);

	Compiler->CreateJmpToBlock(WhileBlock);

	return nullptr;
}

ASTForExpression::ASTForExpression(HazeVM* VM, std::unique_ptr<ASTBase>& InitExpression, std::unique_ptr<ASTBase>& ConditionExpression, std::unique_ptr<ASTBase>& StepExpression
	, std::unique_ptr<ASTBase>& MultiExpression)
	: ASTBase(VM), InitExpression(std::move(InitExpression)), ConditionExpression(std::move(ConditionExpression)), StepExpression(std::move(StepExpression)),
	MultiExpression(std::move(MultiExpression))
{
}

ASTForExpression::~ASTForExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTForExpression::CodeGen()
{
	auto& Compiler = VM->GetCompiler();
	auto Function = Compiler->GetCurrModule()->GetCurrFunction();
	//auto TopBlock = Function->GetTopBaseBlock();
	auto ConditionExp = static_cast<ASTBinaryExpression*>(ConditionExpression.get());

	auto ParentBlock = Compiler->GetInsertBlock();

	auto ForBlock = HazeBaseBlock::CreateBaseBlock(Function->GenForBlockName(), Function, ParentBlock);
	auto ForConditionBlock = HazeBaseBlock::CreateBaseBlock(Function->GenForConditionBlockName(), Function, ForBlock);
	auto ForEndBlock = HazeBaseBlock::CreateBaseBlock(Function->GenForEndBlockName(), Function, ParentBlock);

	Compiler->CreateJmpFromBlock(Compiler->GetInsertBlock(), ForBlock, true);

	Compiler->SetInsertBlock(ForBlock);

	InitExpression->CodeGen();

	Compiler->CreateJmpToBlock(ForConditionBlock);

	Compiler->SetInsertBlock(ForConditionBlock);

	ConditionExpression->CodeGen();

	Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(ConditionExp->OperatorToken), nullptr, ForEndBlock, false);

	MultiExpression->CodeGen();

	StepExpression->CodeGen();

	Compiler->CreateJmpToBlock(ForConditionBlock);

	
	ForBlock->FinishBlock(ForEndBlock, false);
	ForEndBlock->FinishBlock();


	return std::shared_ptr<HazeCompilerValue>();
}
