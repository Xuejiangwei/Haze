#include "ASTBase.h"
#include "HazeLog.h"

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

//Base
ASTBase::ASTBase(HazeCompiler* Compiler, const SourceLocation& Location) : Compiler(Compiler), Location(Location)
{
	DefineVariable.Name = HAZE_TEXT("");
	DefineVariable.Type = { HazeValueType::Void, HAZE_TEXT("") };
}

ASTBase::ASTBase(HazeCompiler* Compiler, const SourceLocation& Location, const HazeDefineVariable& Var) : Compiler(Compiler), DefineVariable(Var), Location(Location)
{
	memset(&Value.Value, 0, sizeof(Value.Value));
}

ASTBase::~ASTBase()
{
}

ASTBool::ASTBool(HazeCompiler* Compiler, const SourceLocation& Location, const HazeValue& InValue) : ASTBase(Compiler, Location)
{
	DefineVariable.Type.PrimaryType = HazeValueType::Bool;
	Value = InValue;
}

ASTBool::~ASTBool()
{
}

std::shared_ptr<HazeCompilerValue> ASTBool::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> RetValue = Compiler->GenConstantValue(DefineVariable.Type.PrimaryType, Value);

	return RetValue;
}

ASTNumber::ASTNumber(HazeCompiler* Compiler, const SourceLocation& Location, HazeValueType Type, const HazeValue& InValue) : ASTBase(Compiler, Location)
{
	DefineVariable.Type.PrimaryType = Type;
	Value = InValue;
}

ASTNumber::~ASTNumber()
{
}

std::shared_ptr<HazeCompilerValue> ASTNumber::CodeGen()
{
	return Compiler->GenConstantValue(DefineVariable.Type.PrimaryType, Value);
}

ASTStringText::ASTStringText(HazeCompiler* Compiler, const SourceLocation& Location, HAZE_STRING& Text) : ASTBase(Compiler, Location), Text(std::move(Text))
{
}

ASTStringText::~ASTStringText()
{
}

std::shared_ptr<HazeCompilerValue> ASTStringText::CodeGen()
{
	return Compiler->GenStringVariable(Text);
}

ASTIdentifier::ASTIdentifier(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, HAZE_STRING& Name, 
	std::vector<std::unique_ptr<ASTBase>>& ArrayIndexExpression)
	: ASTBase(Compiler, Location), SectionSignal(Section), ArrayIndexExpression(std::move(ArrayIndexExpression))
{
	DefineVariable.Name = std::move(Name);
}

ASTIdentifier::~ASTIdentifier()
{
}

std::shared_ptr<HazeCompilerValue> ASTIdentifier::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> RetValue = nullptr;

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
		if (!Compiler->GetCurrModule()->GetFunction(DefineVariable.Name))
		{
			HAZE_LOG_ERR(HAZE_TEXT("未能找到变量<%s>,当前函数<%s>!\n"), DefineVariable.Name.c_str(),
				SectionSignal == HazeSectionSignal::Function ? Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str() : HAZE_TEXT("None"));
			return nullptr;
		}
	}

	return RetValue;
}

ASTFunctionCall::ASTFunctionCall(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, HAZE_STRING& Name, std::vector<std::unique_ptr<ASTBase>>& FunctionParam)
	: ASTBase(Compiler, Location), SectionSignal(Section), Name(std::move(Name)), FunctionParam(std::move(FunctionParam))
{
}

ASTFunctionCall::~ASTFunctionCall()
{
}

std::shared_ptr<HazeCompilerValue> ASTFunctionCall::CodeGen()
{
	auto Function = Compiler->GetFunction(Name);

	std::vector<std::shared_ptr<HazeCompilerValue>> Param;

	for (int i = (int)FunctionParam.size() - 1; i >= 0; i--)
	{
		Param.push_back(FunctionParam[i]->CodeGen());
	}

	Compiler->InsertLineCount(Location.Line);
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

ASTVariableDefine::ASTVariableDefine(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, const HazeDefineVariable& DefineVariable,
	std::unique_ptr<ASTBase> Expression, std::vector<std::unique_ptr<ASTBase>> ArraySize, int PointerLevel, std::vector<HazeDefineType>* Vector_ParamType)
	: ASTBase(Compiler, Location, DefineVariable), SectionSignal(Section), Expression(std::move(Expression)), ArraySize(std::move(ArraySize)), PointerLevel(PointerLevel)
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
	std::unique_ptr<HazeCompilerModule>& Module = Compiler->GetCurrModule();

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
		if (auto NullExpression = dynamic_cast<ASTNullPtr*>(Expression.get()))
		{
			NullExpression->SetDefineType(DefineVariable.Type);
		}

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
		Compiler->InsertLineCount(Location.Line);
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

ASTReturn::ASTReturn(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression)
	:ASTBase(Compiler, Location), Expression(std::move(Expression))
{
}

ASTReturn::~ASTReturn()
{
}

std::shared_ptr<HazeCompilerValue> ASTReturn::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> RetValue = Expression ? Expression->CodeGen() : Compiler->GetConstantValueInt(0);
	Compiler->InsertLineCount(Location.Line);
	return Compiler->CreateRet(RetValue);
}

ASTNew::ASTNew(HazeCompiler* Compiler, const SourceLocation& Location, const HazeDefineVariable& Define) : ASTBase(Compiler, Location, Define)
{
}

ASTNew::~ASTNew()
{
}

std::shared_ptr<HazeCompilerValue> ASTNew::CodeGen()
{
	return Compiler->CreateNew(Compiler->GetCurrModule()->GetCurrFunction(), DefineVariable.Type);
}

ASTGetAddress::ASTGetAddress(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression) : ASTBase(Compiler, Location), Expression(std::move(Expression))
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
		auto Function = Compiler->GetCurrModule()->GetFunction(Expression->GetName());
		if (Function)
		{
			Ret = Compiler->CreatePointerToFunction(Function);
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("未能找到函数<%s>去获得函数地址!\n"), Expression->GetName());
		}
	}
	else
	{
		Ret = Compiler->CreatePointerToValue(Ret);
	}

	return Ret;
}

ASTPointerValue::ASTPointerValue(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression, int Level, std::unique_ptr<ASTBase> AssignExpression)
	: ASTBase(Compiler, Location), Expression(std::move(Expression)), Level(Level), AssignExpression(std::move(AssignExpression))
{
}

ASTPointerValue::~ASTPointerValue()
{
}

std::shared_ptr<HazeCompilerValue> ASTPointerValue::CodeGen()
{
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

ASTNeg::ASTNeg(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression, bool IsNumberNeg) 
	: ASTBase(Compiler, Location), Expression(std::move(Expression)), IsNumberNeg(IsNumberNeg)
{
}

ASTNeg::~ASTNeg()
{
}

std::shared_ptr<HazeCompilerValue> ASTNeg::CodeGen()
{
	if (IsNumberNeg)
	{
		return Compiler->CreateNeg(Expression->CodeGen());
	}
	else
	{
		return Compiler->CreateBitNeg(Expression->CodeGen());
	}
}


ASTNullPtr::ASTNullPtr(HazeCompiler* Compiler, const SourceLocation& Location) : ASTBase(Compiler, Location)
{
}

ASTNullPtr::~ASTNullPtr()
{
}

std::shared_ptr<HazeCompilerValue> ASTNullPtr::CodeGen()
{
	return Compiler->GetNullPtr(DefineVariable.Type);
}

void ASTNullPtr::SetDefineType(const HazeDefineType& Type)
{
	DefineVariable.Type = Type;
}

ASTInc::ASTInc(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression, bool IsPreInc) : ASTBase(Compiler, Location), IsPreInc(IsPreInc), Expression(std::move(Expression))
{
}

ASTInc::~ASTInc()
{
}

std::shared_ptr<HazeCompilerValue> ASTInc::CodeGen()
{
	return Compiler->CreateInc(Expression->CodeGen(), IsPreInc);
}

ASTDec::ASTDec(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression, bool IsPreDec) : ASTBase(Compiler, Location), IsPreDec(IsPreDec), Expression(std::move(Expression))
{
}

ASTDec::~ASTDec()
{
}

std::shared_ptr<HazeCompilerValue> ASTDec::CodeGen()
{
	return Compiler->CreateDec(Expression->CodeGen(), IsPreDec);
}

ASTOperetorAssign::ASTOperetorAssign(HazeCompiler* Compiler, const SourceLocation& Location, HazeToken Token, std::unique_ptr<ASTBase>& Expression) : ASTBase(Compiler, Location), Token(Token), Expression(std::move(Expression))
{
}

ASTOperetorAssign::~ASTOperetorAssign()
{
}

std::shared_ptr<HazeCompilerValue> ASTOperetorAssign::CodeGen()
{
	return Expression->CodeGen();
}

ASTMultiExpression::ASTMultiExpression(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, std::vector<std::unique_ptr<ASTBase>>& VectorExpression)
	: ASTBase(Compiler, Location), SectionSignal(Section), VectorExpression(std::move(VectorExpression))
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

ASTBinaryExpression::ASTBinaryExpression(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, HazeToken OperatorToken, std::unique_ptr<ASTBase>& LeftAST, std::unique_ptr<ASTBase>& RightAST)
	:ASTBase(Compiler, Location), SectionSignal(Section), OperatorToken(OperatorToken), LeftAST(std::move(LeftAST)), RightAST(std::move(RightAST)), 
	LeftBlock(nullptr), RightBlock(nullptr), DafaultBlock(nullptr)
{
}

ASTBinaryExpression::~ASTBinaryExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTBinaryExpression::CodeGen()
{
	Compiler->InsertLineCount(Location.Line);

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
			if (!LeftExp)
			{
				HAZE_LOG_ERR(HAZE_TEXT("二元表达式错误!\n"));
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

ASTThreeExpression::ASTThreeExpression(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& ConditionAST, std::unique_ptr<ASTBase>& LeftAST, std::unique_ptr<ASTBase>& RightAST)
	: ASTBase(Compiler, Location), ConditionAST(std::move(ConditionAST)), LeftAST(std::move(LeftAST)), RightAST(std::move(RightAST))
{
}

ASTThreeExpression::~ASTThreeExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTThreeExpression::CodeGen()
{
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

ASTImportModule::ASTImportModule(HazeCompiler* Compiler, const SourceLocation& Location, const HAZE_STRING& ModuleName) : ASTBase(Compiler, Location), ModuleName(ModuleName)
{
}

ASTImportModule::~ASTImportModule()
{
}

std::shared_ptr<HazeCompilerValue> ASTImportModule::CodeGen()
{
	Compiler->AddImportModuleToCurrModule(Compiler->ParseModule(ModuleName));
	return nullptr;
}

ASTBreakExpression::ASTBreakExpression(HazeCompiler* Compiler, const SourceLocation& Location) : ASTBase(Compiler, Location)
{
}

ASTBreakExpression::~ASTBreakExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTBreakExpression::CodeGen()
{
	Compiler->CreateJmpToBlock(Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopEnd()->GetShared());
	Compiler->InsertLineCount(Location.Line);
	return nullptr;
}

ASTContinueExpression::ASTContinueExpression(HazeCompiler* Compiler, const SourceLocation& Location) : ASTBase(Compiler, Location)
{
}

ASTContinueExpression::~ASTContinueExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTContinueExpression::CodeGen()
{
	Compiler->CreateJmpToBlock(Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopStep()->GetShared());
	Compiler->InsertLineCount(Location.Line);
	return nullptr;
}

ASTIfExpression::ASTIfExpression(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Condition, std::unique_ptr<ASTBase>& IfExpression, std::unique_ptr<ASTBase>& ElseExpression)
	: ASTBase(Compiler, Location), Condition(std::move(Condition)), IfExpression(std::move(IfExpression)), ElseExpression(std::move(ElseExpression))
{
}

ASTIfExpression::~ASTIfExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTIfExpression::CodeGen()
{
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

	Compiler->SetInsertBlock(NextBlock);
	return nullptr;
}

ASTWhileExpression::ASTWhileExpression(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Condition, std::unique_ptr<ASTBase>& MultiExpression)
	:ASTBase(Compiler, Location), Condition(std::move(Condition)), MultiExpression(std::move(MultiExpression))
{
}

ASTWhileExpression::~ASTWhileExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTWhileExpression::CodeGen()
{
	auto Function = Compiler->GetCurrModule()->GetCurrFunction();

	auto ParentBlock = Compiler->GetInsertBlock();
	auto WhileBlock = HazeBaseBlock::CreateBaseBlock(Function->GenWhileBlockName(), Function, ParentBlock);
	auto NextBlock = HazeBaseBlock::CreateBaseBlock(Function->GenWhileBlockName(), Function, ParentBlock);

	WhileBlock->SetLoopEnd(NextBlock.get());
	WhileBlock->SetLoopStep(WhileBlock.get());

	Compiler->InsertLineCount(Location.Line);
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

ASTForExpression::ASTForExpression(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& InitExpression, std::unique_ptr<ASTBase>& ConditionExpression, std::unique_ptr<ASTBase>& StepExpression
	, std::unique_ptr<ASTBase>& MultiExpression)
	: ASTBase(Compiler, Location), InitExpression(std::move(InitExpression)), ConditionExpression(std::move(ConditionExpression)), StepExpression(std::move(StepExpression)),
	MultiExpression(std::move(MultiExpression))
{
}

ASTForExpression::~ASTForExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTForExpression::CodeGen()
{
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

ASTInitializeList::ASTInitializeList(HazeCompiler* Compiler, const SourceLocation& Location, std::vector<std::unique_ptr<ASTBase>>& InitializeListExpression)
	: ASTBase(Compiler, Location), InitializeListExpression(std::move(InitializeListExpression))
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