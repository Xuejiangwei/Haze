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

ASTIdentifier::ASTIdentifier(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HAZE_STRING* MemberName, std::vector<std::unique_ptr<ASTBase>>& ArrayIndexExpression)
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
		if (RetValue->IsArray() || RetValue->IsPointerArray())
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
			HAZE_LOG_ERR(HAZE_TEXT("未能找到变量<%s>,当前函数<%s>!\n"), DefineVariable.Name.c_str(),
				SectionSignal == HazeSectionSignal::Function ? Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str() : HAZE_TEXT("None"));
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

			if (!ThisPointerValue)
			{
				Param.push_back(Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(HAZE_CLASS_THIS));
			}
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
			HAZE_LOG_ERR(HAZE_TEXT("变量<%s>定义失败，定义数组长度必须是常量! 当前函数<%s>\n"), DefineVariable.Name.c_str(),
				SectionSignal == HazeSectionSignal::Function ? Module->GetCurrFunction()->GetName().c_str() : HAZE_TEXT("None"));
			return nullptr;
		}
	}

	std::shared_ptr<HazeCompilerValue> ExprValue = nullptr;
	if (Expression)
	{
		ExprValue = Expression->CodeGen();
		if (!ExprValue)
		{
			return nullptr;
		}
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
	else if (SectionSignal == HazeSectionSignal::Class)
	{
		RetValue = Compiler->CreateClassVariable(Module, DefineVariable, ExprValue, SizeValue, &Vector_PointerFunctionParamType);
	}

	if (RetValue && ExprValue)
	{
		if (ArraySize.size() > 0 && RetValue->IsArray())
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
	std::shared_ptr<HazeCompilerValue> RetValue = Expression ? Expression->CodeGen() : VM->GetCompiler()->GetConstantValueInt(0);
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
			HAZE_LOG_ERR(HAZE_TEXT("未能找到函数<%s>去获得函数地址!\n"), Expression->GetName());
		}
	}
	else
	{
		Ret = VM->GetCompiler()->CreatePointerToValue(Ret);
	}

	return Ret;
}

ASTPointerValue::ASTPointerValue(HazeVM* VM, std::unique_ptr<ASTBase>& Expression, int Level, std::unique_ptr<ASTBase> AssignExpression) 
	: ASTBase(VM), Expression(std::move(Expression)), Level(Level), AssignExpression(std::move(AssignExpression))
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
		if (AssignExpression)
		{
			return Compiler->CreateMovToPV(PointerValue, AssignExpression->CodeGen());
		}

		return Compiler->CreateMovPV(Compiler->GetTempRegister(), PointerValue);
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("未能获得<%s>指针指向的值!\n"), DefineVariable.Name.c_str());
	}

	return nullptr;
}

ASTNeg::ASTNeg(HazeVM* VM, std::unique_ptr<ASTBase>& Expression, bool IsNumberNeg) : ASTBase(VM), Expression(std::move(Expression)), IsNumberNeg(IsNumberNeg)
{
}

ASTNeg::~ASTNeg()
{
}

std::shared_ptr<HazeCompilerValue> ASTNeg::CodeGen()
{
	if (IsNumberNeg)
	{
		return VM->GetCompiler()->CreateNeg(Expression->CodeGen());
	}
	else
	{
		return VM->GetCompiler()->CreateBitNeg(Expression->CodeGen());
	}
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
	:ASTBase(VM), SectionSignal(Section), OperatorToken(OperatorToken), LeftAST(std::move(LeftAST)), RightAST(std::move(RightAST)), 
	LeftBlock(nullptr), RightBlock(nullptr), DafaultBlock(nullptr)
{
}

ASTBinaryExpression::~ASTBinaryExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTBinaryExpression::CodeGen()
{
	auto RightExp = dynamic_cast<ASTBinaryExpression*>(RightAST.get());
	if (RightExp && IsAndOrToken(RightExp->OperatorToken))
	{
		RightExp->SetLeftAndRightBlock(LeftBlock, RightBlock);
	}

	auto LeftExp = dynamic_cast<ASTBinaryExpression*>(LeftAST.get());
	if (LeftExp && IsAndOrToken(LeftExp->OperatorToken))
	{
		LeftExp->SetLeftAndRightBlock(LeftBlock, RightBlock);
	}

	auto& Compiler = VM->GetCompiler();

	switch (OperatorToken)
	{
	case HazeToken::Add:
		return Compiler->CreateAdd(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::Sub:
		return Compiler->CreateSub(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::Mul:
		return Compiler->CreateMul(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::Div:
		return Compiler->CreateDiv(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::Mod:
		return Compiler->CreateMod(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::And:
	{
		auto Function = Compiler->GetCurrModule()->GetCurrFunction();
		auto Ret = LeftAST->CodeGen();
		if (!LeftExp)
		{
			Compiler->CreateBoolCmp(Ret);
		}

		if (RightExp)
		{
			Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(OperatorToken), nullptr,
				DafaultBlock ? DafaultBlock->GetShared() : RightBlock->GetShared());

			RightAST->CodeGen();

			Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(OperatorToken), LeftBlock ? LeftBlock->GetShared() : nullptr,
				DafaultBlock ? DafaultBlock->GetShared() : RightBlock->GetShared());
			
		}
		else
		{
			//Ret = Compiler->CreateAnd(Ret, RightAST->CodeGen());
			if (!LeftExp)
			{
				HAZE_LOG_ERR(HAZE_TEXT("二元表达式错误!\n"));
				//LeftExp->SetLeftAndRightBlock(LeftBlock, RightBlock);
				return nullptr;
			}

			Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(LeftExp->OperatorToken), nullptr, RightBlock->GetShared());
			Compiler->CreateBoolCmp(RightAST->CodeGen());
			Compiler->CreateCompareJmp(HazeCmpType::Equal, LeftBlock ? LeftBlock->GetShared() : nullptr,
				DafaultBlock ? DafaultBlock->GetShared() : RightBlock->GetShared());
		}
		return Ret;
	}
	case HazeToken::Or:
	{
		auto Function = Compiler->GetCurrModule()->GetCurrFunction();

		auto Ret = LeftAST->CodeGen();
		
		if (LeftExp && IsAndToken(LeftExp->OperatorToken))
		{
			auto Block = HazeBaseBlock::CreateBaseBlock(Function->GenDafaultBlockName(), Function, Compiler->GetInsertBlock());
			DafaultBlock = Block.get();
			LeftExp->SetLeftAndRightBlock(LeftBlock, DafaultBlock);
		}

		if (DafaultBlock)
		{
			Compiler->SetInsertBlock(DafaultBlock->GetShared());
		}
		else
		{
			if (!LeftExp)
			{
				Compiler->CreateBoolCmp(Ret);
			}
			else
			{
				Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(OperatorToken), LeftBlock ? LeftBlock->GetShared() : nullptr, nullptr);
			}
		}

		RightAST->CodeGen();
		
		return Ret;
	}
	case HazeToken::Not:
		return Compiler->CreateNot(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::Shl:
		return Compiler->CreateShl(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::Shr:
		return Compiler->CreateShr(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::Assign:
		return Compiler->CreateMov(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::BitAnd:
		return Compiler->CreateBitAnd(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::BitOr:
		return Compiler->CreateBitOr(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::BitXor:
		return Compiler->CreateBitXor(LeftAST->CodeGen(), RightAST->CodeGen());
	case HazeToken::Equal:
	case HazeToken::NotEqual:
	case HazeToken::Greater:
	case HazeToken::GreaterEqual:
	case HazeToken::Less:
	case HazeToken::LessEqual:
	{
		auto Ret = Compiler->CreateIntCmp(LeftAST->CodeGen(), RightAST->CodeGen());
		if (LeftBlock || RightBlock)
		{
			Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(OperatorToken), LeftBlock ? LeftBlock->GetShared() : nullptr,
				RightBlock ? RightBlock->GetShared() : nullptr);
		}

		return Ret;
	}
	case HazeToken::AddAssign:
		return Compiler->CreateAdd(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	case HazeToken::SubAssign:
		return Compiler->CreateSub(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	case HazeToken::MulAssign:
		return Compiler->CreateMul(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	case HazeToken::DivAssign:
		return Compiler->CreateDiv(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	case HazeToken::ModAssign:
		return Compiler->CreateMod(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	case HazeToken::BitAndAssign:
		return Compiler->CreateBitAnd(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	case HazeToken::BitOrAssign:
		return Compiler->CreateBitOr(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	case HazeToken::BitXorAssign:
		return Compiler->CreateBitXor(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	case HazeToken::ShlAssign:
		return Compiler->CreateShl(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	case HazeToken::ShrAssign:
		return Compiler->CreateShr(LeftAST->CodeGen(), RightAST->CodeGen(), true);
	default:
		break;
	}

	return nullptr;
}

void ASTBinaryExpression::SetLeftAndRightBlock(HazeBaseBlock* LeftJmpBlock, HazeBaseBlock* RightJmpBlock)
{
	LeftBlock = LeftJmpBlock;
	RightBlock = RightJmpBlock;
}

ASTThreeExpression::ASTThreeExpression(HazeVM* VM, std::unique_ptr<ASTBase>& ConditionAST, std::unique_ptr<ASTBase>& LeftAST, std::unique_ptr<ASTBase>& RightAST)
	: ASTBase(VM), ConditionAST(std::move(ConditionAST)), LeftAST(std::move(LeftAST)), RightAST(std::move(RightAST))
{
}

ASTThreeExpression::~ASTThreeExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTThreeExpression::CodeGen()
{
	auto& Compiler = VM->GetCompiler();
	auto Function = Compiler->GetCurrModule()->GetCurrFunction();

	auto ParentBlock = Compiler->GetInsertBlock();

	auto DefauleBlock = HazeBaseBlock::CreateBaseBlock(Function->GenDafaultBlockName(), Function, ParentBlock);

	auto BlockLeft = HazeBaseBlock::CreateBaseBlock(Function->GenDafaultBlockName(), Function, ParentBlock);
	auto BlockRight = HazeBaseBlock::CreateBaseBlock(Function->GenDafaultBlockName(), Function, ParentBlock);

	auto ConditionExp = dynamic_cast<ASTBinaryExpression*>(ConditionAST.get());
	if (ConditionExp)
	{
		ConditionExp->CodeGen();
		Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(ConditionExp->OperatorToken), BlockLeft, BlockRight);
	}
	else
	{
		Compiler->CreateBoolCmp(ConditionAST->CodeGen());
		Compiler->CreateCompareJmp(HazeCmpType::Equal, BlockLeft, BlockRight);
	}

	auto TempValue = HazeCompiler::GetTempRegister();
	Compiler->SetInsertBlock(BlockLeft);
	Compiler->CreateMov(TempValue, LeftAST->CodeGen());
	Compiler->CreateJmpToBlock(DefauleBlock);

	Compiler->SetInsertBlock(BlockRight);
	Compiler->CreateMov(TempValue, RightAST->CodeGen());
	Compiler->CreateJmpToBlock(DefauleBlock);

	Compiler->SetInsertBlock(DefauleBlock);

	return TempValue;
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

ASTBreakExpression::ASTBreakExpression(HazeVM* VM) : ASTBase(VM)
{
}

ASTBreakExpression::~ASTBreakExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTBreakExpression::CodeGen()
{
	VM->GetCompiler()->CreateJmpToBlock(VM->GetCompiler()->GetInsertBlock()->FindLoopBlock()->GetLoopEnd()->GetShared());
	return nullptr;
}

ASTContinueExpression::ASTContinueExpression(HazeVM* VM) : ASTBase(VM)
{
}

ASTContinueExpression::~ASTContinueExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTContinueExpression::CodeGen()
{
	VM->GetCompiler()->CreateJmpToBlock(VM->GetCompiler()->GetInsertBlock()->FindLoopBlock()->GetLoopStep()->GetShared());
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
	auto& Compiler = VM->GetCompiler();
	auto Function = Compiler->GetCurrModule()->GetCurrFunction();

	auto ParentBlock = Compiler->GetInsertBlock();
	auto IfThenBlock = HazeBaseBlock::CreateBaseBlock(Function->GenIfThenBlockName(), Function, ParentBlock);
	auto ElseBlock = ElseExpression ? HazeBaseBlock::CreateBaseBlock(Function->GenElseBlockName(), Function, ParentBlock) : nullptr;
	auto NextBlock = HazeBaseBlock::CreateBaseBlock(Function->GenDafaultBlockName(), Function, ParentBlock);

	auto ConditionExp = dynamic_cast<ASTBinaryExpression*>(Condition.get());

	if (ConditionExp && GetHazeCmpTypeByToken(ConditionExp->OperatorToken) != HazeCmpType::None)
	{
		ConditionExp->SetLeftAndRightBlock(IfThenBlock.get(), ElseExpression ? ElseBlock.get() : NextBlock.get());
		Condition->CodeGen();
	}
	else
	{
		Compiler->CreateBoolCmp(Condition->CodeGen());
		Compiler->CreateCompareJmp(HazeCmpType::Equal, IfThenBlock, ElseExpression ? ElseBlock : NextBlock);
	}

	Compiler->SetInsertBlock(IfThenBlock);
	IfExpression->CodeGen();
	Compiler->CreateJmpToBlock(NextBlock);

	if (ElseExpression)
	{
		Compiler->SetInsertBlock(ElseBlock);
		ElseExpression->CodeGen();
		Compiler->CreateJmpToBlock(NextBlock);
	}
	else
	{
		//Compiler->SetInsertBlock(InsertBlock);
	}

	Compiler->SetInsertBlock(NextBlock);
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

	auto ParentBlock = Compiler->GetInsertBlock()->GetInsertToBlock()->GetShared();
	auto WhileBlock = HazeBaseBlock::CreateBaseBlock(Function->GenWhileBlockName(), Function, ParentBlock);
	auto NextBlock = HazeBaseBlock::CreateBaseBlock(Function->GenWhileBlockName(), Function, ParentBlock);

	WhileBlock->SetLoopEnd(NextBlock.get());
	WhileBlock->SetLoopStep(WhileBlock.get());

	auto ConditionExp = dynamic_cast<ASTBinaryExpression*>(Condition.get());
	if (ConditionExp)
	{
		ConditionExp->SetLeftAndRightBlock(WhileBlock.get(), NextBlock.get());
		Condition->CodeGen();
	}
	else
	{
		Compiler->CreateBoolCmp(Condition->CodeGen());
		Compiler->CreateCompareJmp(HazeCmpType::Equal, WhileBlock, NextBlock);
	}

	Compiler->SetInsertBlock(WhileBlock);
	MultiExpression->CodeGen();
	Compiler->CreateJmpToBlock(WhileBlock);
	
	Compiler->SetInsertBlock(NextBlock);
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
	auto ParentBlock = Compiler->GetInsertBlock();

	auto ForConditionBlock = HazeBaseBlock::CreateBaseBlock(Function->GenForConditionBlockName(), Function, ParentBlock);
	auto LoopBlock = HazeBaseBlock::CreateBaseBlock(Function->GenLoopBlockName(), Function, ParentBlock);
	auto ForStepBlock = HazeBaseBlock::CreateBaseBlock(Function->GenForStepBlockName(), Function, ParentBlock);
	auto EndLoopBlock = HazeBaseBlock::CreateBaseBlock(Function->GenDafaultBlockName(), Function, ParentBlock);
	LoopBlock->SetLoopEnd(EndLoopBlock.get());
	LoopBlock->SetLoopStep(ForStepBlock.get());

	if (!InitExpression->CodeGen())
	{
		HAZE_LOG_ERR(HAZE_TEXT("循环语句初始化失败!\n"));
		return nullptr;
	}

	Compiler->CreateJmpToBlock(ForConditionBlock);
	Compiler->SetInsertBlock(ForConditionBlock);
	auto ConditionExp = dynamic_cast<ASTBinaryExpression*>(ConditionExpression.get());
	if (ConditionExp)
	{
		ConditionExp->SetLeftAndRightBlock(LoopBlock.get(), EndLoopBlock.get());
		ConditionExpression->CodeGen();
	}
	else
	{
		Compiler->CreateBoolCmp(ConditionExpression->CodeGen());
		Compiler->CreateCompareJmp(HazeCmpType::Equal, LoopBlock, EndLoopBlock);
	}

	Compiler->SetInsertBlock(LoopBlock);
	MultiExpression->CodeGen();
	Compiler->CreateJmpToBlock(ForStepBlock);

	Compiler->SetInsertBlock(ForStepBlock);
	StepExpression->CodeGen();
	Compiler->CreateJmpToBlock(ForConditionBlock);

	Compiler->SetInsertBlock(EndLoopBlock);

	return nullptr;
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

	auto InitilaizeListValue = HazeCompiler::GetInitializeListValue();
	InitilaizeListValue->ResetInitializeList(Vector_Value);

	return InitilaizeListValue;
}


