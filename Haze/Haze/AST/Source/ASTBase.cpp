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
#include "HazeCompilerInitListValue.h"
#include "HazeCompilerHelper.h"

#include "ASTBase.h"

#include "HazeLog.h"

//Base
ASTBase::ASTBase(HazeVM* VM) : VM(VM)
{
	DefineVariable.Name = HAZE_TEXT("");
	DefineVariable.Type = { HazeValueType::Void, HAZE_TEXT("") };
}

ASTBase::ASTBase(HazeVM* VM, const HazeDefineVariable& Var) : VM(VM), DefineVariable(Var)
{
	memset(&Value.Value, 0, sizeof(Value.Value));
}

ASTBase::~ASTBase()
{
}

ASTBool::ASTBool(HazeVM* VM, const HazeValue& InValue) : ASTBase(VM)
{
	DefineVariable.Type.PrimaryType = HazeValueType::Bool;
	Value = InValue;
}

ASTBool::~ASTBool()
{
}

std::shared_ptr<HazeCompilerValue> ASTBool::CodeGen()
{
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	std::shared_ptr<HazeCompilerValue> RetValue = Compiler->GenConstantValue(DefineVariable.Type.PrimaryType, Value);

	return RetValue;
}

ASTNumber::ASTNumber(HazeVM* VM, HazeValueType Type, const HazeValue& InValue) : ASTBase(VM)
{
	DefineVariable.Type.PrimaryType = Type;
	Value = InValue;
}

ASTNumber::~ASTNumber()
{
}

std::shared_ptr<HazeCompilerValue> ASTNumber::CodeGen()
{
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	std::shared_ptr<HazeCompilerValue> RetValue = Compiler->GenConstantValue(DefineVariable.Type.PrimaryType, Value);

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

ASTIdentifier::ASTIdentifier(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HAZE_STRING* MemberName, std::vector<std::unique_ptr<ASTBase>> ArrayIndexExpression)
	: ASTBase(VM), SectionSignal(Section), ArrayIndexExpression(std::move(ArrayIndexExpression))
{
	DefineVariable.Name = std::move(Name);
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
	auto& Compiler = VM->GetCompiler();

	if (SectionSignal == HazeSectionSignal::Global)
	{
		RetValue = Compiler->GetGlobalVariable(DefineVariable.Name);
	}
	else if (SectionSignal == HazeSectionSignal::Function)
	{
		RetValue = Compiler->GetLocalVariable(DefineVariable.Name);
		if (!RetValue)
		{
			RetValue = Compiler->GetGlobalVariable(DefineVariable.Name);
		}
	}

	if (RetValue)
	{
		if (RetValue->IsArray())
		{
			if (ArrayIndexExpression.size() > 0)
			{
				std::vector<std::shared_ptr<HazeCompilerValue>> IndexValue;
				for (size_t i = 0; i < ArrayIndexExpression.size(); i++)
				{
					IndexValue.push_back(ArrayIndexExpression[i]->CodeGen());
				}

				RetValue = Compiler->CreateArrayElement(RetValue, IndexValue);
			}
			else
			{
				RetValue = Compiler->CreatePointerToArray(RetValue);
			}
		}
	}
	else
	{
		if (!VM->GetCompiler()->GetCurrModule()->GetFunction(DefineVariable.Name))
		{
			HAZE_LOG_ERR(HAZE_TEXT("未能找到变量<%s>!\n"), DefineVariable.Name.c_str());
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

	std::vector<std::shared_ptr<HazeCompilerValue>> Param;

	for (int i = (int)FunctionParam.size() - 1; i >= 0; i--)
	{
		Param.push_back(FunctionParam[i]->CodeGen());
	}

	if (Function)
	{
		std::shared_ptr<HazeCompilerValue> ThisPointerValue = nullptr;
		if (Function->GetClass())
		{
			ThisPointerValue = Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(GetObjectName(Name));
		}

		return Compiler->CreateFunctionCall(Function, Param, ThisPointerValue);
	}
	else
	{
		//函数指针
		auto FunctionPointer = Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(Name);
		return Compiler->CreateFunctionCall(FunctionPointer, Param);
	}
}

ASTVariableDefine::ASTVariableDefine(HazeVM* VM, HazeSectionSignal Section, const HazeDefineVariable& DefineVariable, std::unique_ptr<ASTBase> Expression,
	std::vector<std::unique_ptr<ASTBase>> ArraySize, int PointerLevel, std::vector<HazeDefineType>* Vector_ParamType)
	: ASTBase(VM, DefineVariable), SectionSignal(Section), Expression(std::move(Expression)), ArraySize(std::move(ArraySize)), PointerLevel(PointerLevel)
{
	if (Vector_ParamType)
	{
		Vector_PointerFunctionParamType = std::move(*Vector_ParamType);
	}
}

ASTVariableDefine::~ASTVariableDefine()
{
}

std::shared_ptr<HazeCompilerValue> ASTVariableDefine::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> RetValue = nullptr;
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	std::unique_ptr<HazeCompilerModule>& Module = VM->GetCompiler()->GetCurrModule();

	std::vector<std::shared_ptr<HazeCompilerValue>> SizeValue;
	for (auto& Iter : ArraySize)
	{
		auto V = Iter->CodeGen();
		if (V->IsConstant())
		{
			SizeValue.push_back(V);
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("变量定义失败，定义数组长度必须是常量！\n"));
			return nullptr;
		}
	}

	std::shared_ptr<HazeCompilerValue> ExprValue = nullptr;
	if (Expression)
	{
		ExprValue = Expression->CodeGen();
	}

	bool IsRef = DefineVariable.Type.PrimaryType == HazeValueType::ReferenceBase || DefineVariable.Type.PrimaryType == HazeValueType::ReferenceClass;

	if (SectionSignal == HazeSectionSignal::Global)
	{
		RetValue = Compiler->CreateGlobalVariable(Module, DefineVariable, ExprValue, SizeValue, &Vector_PointerFunctionParamType);
	}
	else if (SectionSignal == HazeSectionSignal::Function)
	{
		RetValue = Compiler->CreateLocalVariable(Module->GetCurrFunction(), DefineVariable, ExprValue, SizeValue, &Vector_PointerFunctionParamType);
	}

	if (RetValue && ExprValue)
	{
		if (ArraySize.size() > 0)
		{
			Compiler->CreateArrayInit(RetValue, ExprValue);
		}
		else if (IsRef)
		{
			Compiler->CreateLea(RetValue, ExprValue);
		}
		else
		{
			Compiler->CreateMov(RetValue, ExprValue);
		}
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

ASTGetAddress::ASTGetAddress(HazeVM* VM, std::unique_ptr<ASTBase>& Expression) : ASTBase(VM), Expression(std::move(Expression))
{
}

ASTGetAddress::~ASTGetAddress()
{
}

std::shared_ptr<HazeCompilerValue> ASTGetAddress::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> Ret = nullptr;
	if (Expression)
	{
		Ret = Expression->CodeGen();
	}
	
	if (!Ret)
	{
		//获得函数地址
		auto Function = VM->GetCompiler()->GetCurrModule()->GetFunction(Expression->GetName());
		if (Function)
		{
			Ret = VM->GetCompiler()->CreatePointerToFunction(Function);
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("未能找到函数<%s>并获得函数地址!\n"), Expression->GetName());
		}
	}
	else
	{
		Ret = VM->GetCompiler()->CreatePointerToValue(Ret);
	}

	return Ret;
}

ASTPointerValue::ASTPointerValue(HazeVM* VM, std::unique_ptr<ASTBase>& Expression, int Level) : ASTBase(VM), Expression(std::move(Expression)), Level(Level)
{
}

ASTPointerValue::~ASTPointerValue()
{
}

std::shared_ptr<HazeCompilerValue> ASTPointerValue::CodeGen()
{
	auto& Compiler = VM->GetCompiler();
	auto PointerValue = Expression->CodeGen();

	if (PointerValue)
	{
		return Compiler->CreateMovPV(Compiler->GetTempRegister(), PointerValue);
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("Parse pointer value not get pointer value %s"), DefineVariable.Name.c_str());
	}

	return nullptr;
}

ASTNeg::ASTNeg(HazeVM* VM, std::unique_ptr<ASTBase>& Expression) : ASTBase(VM), Expression(std::move(Expression))
{
}

ASTNeg::~ASTNeg()
{
}

std::shared_ptr<HazeCompilerValue> ASTNeg::CodeGen()
{
	return VM->GetCompiler()->CreateBitNeg(Expression->CodeGen());
}

ASTInc::ASTInc(HazeVM* VM, std::unique_ptr<ASTBase>& Expression, bool IsPreInc) : ASTBase(VM), IsPreInc(IsPreInc), Expression(std::move(Expression))
{
}

ASTInc::~ASTInc()
{
}

std::shared_ptr<HazeCompilerValue> ASTInc::CodeGen()
{
	return VM->GetCompiler()->CreateInc(Expression->CodeGen(), IsPreInc);
}

ASTDec::ASTDec(HazeVM* VM, std::unique_ptr<ASTBase>& Expression, bool IsPreDec) : ASTBase(VM), IsPreDec(IsPreDec), Expression(std::move(Expression))
{
}

ASTDec::~ASTDec()
{
}

std::shared_ptr<HazeCompilerValue> ASTDec::CodeGen()
{
	return VM->GetCompiler()->CreateDec(Expression->CodeGen(), IsPreDec);
}

ASTOperetorAssign::ASTOperetorAssign(HazeVM* VM, HazeToken Token, std::unique_ptr<ASTBase>& Expression) : ASTBase(VM), Token(Token), Expression(std::move(Expression))
{
}

ASTOperetorAssign::~ASTOperetorAssign()
{
}

std::shared_ptr<HazeCompilerValue> ASTOperetorAssign::CodeGen()
{
	return Expression->CodeGen();
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
	for (size_t i = 0; i < VectorExpression.size(); i++)
	{
		VectorExpression[i]->CodeGen();
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
	case HazeToken::Sub:
		return Compiler->CreateSub(LeftValue, RightValue);
	case HazeToken::Mul:
		return Compiler->CreateMul(LeftValue, RightValue);
	case HazeToken::Div:
		return Compiler->CreateDiv(LeftValue, RightValue);
	case HazeToken::Mod:
		return Compiler->CreateMod(LeftValue, RightValue);
	case HazeToken::And:
		return Compiler->CreateAnd(LeftValue, RightValue);
	case HazeToken::Or:
		return Compiler->CreateOr(LeftValue, RightValue);
	case HazeToken::Not:
		return Compiler->CreateNot(LeftValue, RightValue);
	case HazeToken::Shl:
		return Compiler->CreateShl(LeftValue, RightValue);
	case HazeToken::Shr:
		return Compiler->CreateShr(LeftValue, RightValue);
	case HazeToken::Assign:
		return Compiler->CreateMov(LeftValue, RightValue);
	case HazeToken::BitAnd:
		return Compiler->CreateBitAnd(LeftValue, RightValue);
	case HazeToken::BitOr:
		return Compiler->CreateBitOr(LeftValue, RightValue);
	case HazeToken::BitXor:
		return Compiler->CreateBitXor(LeftValue, RightValue);
	case HazeToken::Equal:
	case HazeToken::NotEqual:
	case HazeToken::Greater:
	case HazeToken::GreaterEqual:
	case HazeToken::Less:
	case HazeToken::LessEqual:
		return Compiler->CreateIntCmp(LeftValue, RightValue);
	case HazeToken::AddAssign:
		Compiler->CreateAdd(LeftValue, RightValue, true);
		return LeftValue;
	case HazeToken::SubAssign:
		Compiler->CreateSub(LeftValue, RightValue, true);
		return LeftValue;
	case HazeToken::MulAssign:
		Compiler->CreateMul(LeftValue, RightValue, true);
		return LeftValue;
	case HazeToken::DivAssign:
		Compiler->CreateDiv(LeftValue, RightValue, true);
		return LeftValue;
	case HazeToken::ModAssign:
		Compiler->CreateMod(LeftValue, RightValue, true);
		return LeftValue;
	case HazeToken::BitAndAssign:
		Compiler->CreateBitAnd(LeftValue, RightValue, true);
		return LeftValue;
	case HazeToken::BitOrAssign:
		Compiler->CreateBitOr(LeftValue, RightValue, true);
		return LeftValue;
	case HazeToken::BitXorAssign:
		Compiler->CreateBitXor(LeftValue, RightValue, true);
		return LeftValue;
	case HazeToken::ShlAssign:
		Compiler->CreateShl(LeftValue, RightValue, true);
		return LeftValue;
	case HazeToken::ShrAssign:
		Compiler->CreateShr(LeftValue, RightValue, true);
		return LeftValue;
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
	VM->GetCompiler()->AddImportModuleToCurrModule(VM->ParseModule(ModuleName));
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

	auto ParentBlock = Compiler->GetInsertBlock();
	auto IfBlock = HazeBaseBlock::CreateBaseBlock(Function->GenIfBlockName(), Function, ParentBlock);
	auto ElseBlock = HazeBaseBlock::CreateBaseBlock(Function->GenElseBlockName(), Function, ParentBlock);

	Compiler->CreateJmpFromBlock(ParentBlock, IfBlock, true);

	Compiler->SetInsertBlock(IfBlock);

	Condition->CodeGen();

	Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(ConditionExp->OperatorToken), nullptr, ElseBlock, false);

	IfExpression->CodeGen();
	IfBlock->FinishBlock(nullptr);

	Compiler->SetInsertBlock(ElseBlock);
	ElseExpression->CodeGen();
	ElseBlock->FinishBlock(nullptr);

	Compiler->SetInsertBlock(ParentBlock);

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

	auto ParentBlock = Compiler->GetInsertBlock();
	auto WhileBlock = HazeBaseBlock::CreateBaseBlock(Function->GenWhileBlockName(), Function, ParentBlock);

	Compiler->CreateJmpFromBlock(ParentBlock, WhileBlock, true);

	Compiler->SetInsertBlock(WhileBlock);
	ConditionExp->CodeGen();

	Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(ConditionExp->OperatorToken), nullptr, nullptr, false, true);

	MultiExpression->CodeGen();

	Compiler->CreateJmpToBlock(WhileBlock);

	WhileBlock->FinishBlock(nullptr, false);

	Compiler->SetInsertBlock(ParentBlock);

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

	Compiler->CreateJmpFromBlock(ParentBlock, ForBlock, true);

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

	Compiler->SetInsertBlock(ParentBlock);

	return std::shared_ptr<HazeCompilerValue>();
}

ASTInitializeList::ASTInitializeList(HazeVM* VM, std::vector<std::unique_ptr<ASTBase>>& InitializeListExpression)
	: ASTBase(VM), InitializeListExpression(std::move(InitializeListExpression))
{
}

ASTInitializeList::~ASTInitializeList()
{
}

std::shared_ptr<HazeCompilerValue> ASTInitializeList::CodeGen()
{
	std::vector<std::shared_ptr<HazeCompilerValue>> Vector_Value;
	for (size_t i = 0; i < InitializeListExpression.size(); i++)
	{
		Vector_Value.push_back(InitializeListExpression[i]->CodeGen());
	}

	auto InitilaizeListValue = VM->GetCompiler()->GetInitializeListValue();
	InitilaizeListValue->ResetInitializeList(Vector_Value);

	return InitilaizeListValue;
}
