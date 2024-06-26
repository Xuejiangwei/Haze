#include "ASTBase.h"
#include "HazeLogDefine.h"

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
ASTBase::ASTBase(HazeCompiler* compiler, const SourceLocation& location) : m_Compiler(compiler), m_Location(location)
{
	m_DefineVariable.Name = HAZE_TEXT("");
	m_DefineVariable.Type = { HazeValueType::Void, HAZE_TEXT("") };
}

ASTBase::ASTBase(HazeCompiler* compiler, const SourceLocation& location, const HazeDefineVariable& var) 
	: m_Compiler(compiler), m_DefineVariable(var), m_Location(location)
{
	memset(&m_Value.Value, 0, sizeof(m_Value.Value));
}

ASTBase::~ASTBase()
{
}

ASTBool::ASTBool(HazeCompiler* compiler, const SourceLocation& location, const HazeValue& value)
	: ASTBase(compiler, location)
{
	m_DefineVariable.Type.PrimaryType = HazeValueType::Bool;
	m_Value = value;
}

ASTBool::~ASTBool()
{
}

std::shared_ptr<HazeCompilerValue> ASTBool::CodeGen()
{
	return m_Compiler->GenConstantValue(m_DefineVariable.Type.PrimaryType, m_Value);
}

ASTNumber::ASTNumber(HazeCompiler* compiler, const SourceLocation& location, HazeValueType type, const HazeValue& value) 
	: ASTBase(compiler, location)
{
	m_DefineVariable.Type.PrimaryType = type;
	m_Value = value;
}

ASTNumber::~ASTNumber()
{
}

std::shared_ptr<HazeCompilerValue> ASTNumber::CodeGen()
{
	return m_Compiler->GenConstantValue(m_DefineVariable.Type.PrimaryType, m_Value);
}

ASTStringText::ASTStringText(HazeCompiler* compiler, const SourceLocation& location, HAZE_STRING& text) 
	: ASTBase(compiler, location), m_Text(std::move(text))
{
}

ASTStringText::~ASTStringText()
{
}

std::shared_ptr<HazeCompilerValue> ASTStringText::CodeGen()
{
	return m_Compiler->GenStringVariable(m_Text);
}

ASTIdentifier::ASTIdentifier(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, HAZE_STRING& name,
	std::vector<std::unique_ptr<ASTBase>>& arrayIndexExpression)
	: ASTBase(compiler, location), m_SectionSignal(section), m_ArrayIndexExpression(std::move(arrayIndexExpression))
{
	m_DefineVariable.Name = std::move(name);
}

ASTIdentifier::~ASTIdentifier()
{
}

std::shared_ptr<HazeCompilerValue> ASTIdentifier::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> retValue = nullptr;
	HazeVariableScope classMemberScope = HazeVariableScope::Local;

	if (m_SectionSignal == HazeSectionSignal::Global)
	{
		retValue = m_Compiler->GetGlobalVariable(m_DefineVariable.Name);

		classMemberScope = HazeVariableScope::Global;
	}
	else if (m_SectionSignal == HazeSectionSignal::Local)
	{
		retValue = m_Compiler->GetLocalVariable(m_DefineVariable.Name);
		classMemberScope = HazeVariableScope::Local;
		if (!retValue)
		{
			retValue = m_Compiler->GetGlobalVariable(m_DefineVariable.Name);
			classMemberScope = HazeVariableScope::Global;
		}
	}

	if (retValue)
	{
		if (retValue->IsClassMember())
		{
			retValue->SetScope(classMemberScope);
		}

		if (retValue->IsArray() && m_ArrayIndexExpression.size() > 0)
		{
			std::vector<std::shared_ptr<HazeCompilerValue>> indexValue;
			for (size_t i = 0; i < m_ArrayIndexExpression.size(); i++)
			{
				indexValue.push_back(m_ArrayIndexExpression[i]->CodeGen());
			}

			retValue = m_Compiler->CreateArrayElement(retValue, indexValue);
		}
	}
	else
	{
		auto function = m_Compiler->GetCurrModule()->GetFunction(m_DefineVariable.Name);
		if (!function.first)
		{
			HAZE_LOG_ERR_W("未能找到变量<%s>,当前函数<%s>!\n", m_DefineVariable.Name.c_str(),
				m_SectionSignal == HazeSectionSignal::Local ? m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str() : HAZE_TEXT("None"));
			return nullptr;
		}
	}

	return retValue;
}

ASTFunctionCall::ASTFunctionCall(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	HAZE_STRING& name, std::vector<std::unique_ptr<ASTBase>>& functionParam)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Name(std::move(name)), m_FunctionParam(std::move(functionParam))
{
}

ASTFunctionCall::~ASTFunctionCall()
{
}

std::shared_ptr<HazeCompilerValue> ASTFunctionCall::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto [function, objectVariable] = m_Compiler->GetFunction(m_Name);

	std::vector<std::shared_ptr<HazeCompilerValue>> param;

	for (int i = (int)m_FunctionParam.size() - 1; i >= 0; i--)
	{
		param.push_back(m_FunctionParam[i]->CodeGen());
		if (!param.back())
		{
			HAZE_LOG_ERR_W("函数<%s>中<%d>行调用<%s>第<%d>个参数错误!\n", m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str(), m_Location.Line, m_Name.c_str(), i + 1);
			return nullptr;
		}
	}
	
	if (function)
	{
		if (function->GetClass())
		{
			if (!objectVariable)
			{
				param.push_back(m_Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(HAZE_CLASS_THIS));
			}
		}

		return m_Compiler->CreateFunctionCall(function, param, objectVariable);
	}
	else
	{
		//函数指针
		auto functionPointer = m_Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(m_Name);
		if (functionPointer)
		{
			return m_Compiler->CreateFunctionCall(functionPointer, param);
		}
	}

	HAZE_LOG_ERR_W("生成函数调用<%s>错误\n", m_Name.c_str());
	return nullptr;
}

ASTVariableDefine::ASTVariableDefine(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	const HazeDefineVariable& defineVar, std::unique_ptr<ASTBase> expression, std::vector<std::unique_ptr<ASTBase>> arraySize,
	int pointerLevel, std::vector<HazeDefineType>* paramType)
	: ASTBase(compiler, location, defineVar), m_SectionSignal(section), m_Expression(std::move(expression)), 
		m_ArraySize(std::move(arraySize)), m_PointerLevel(pointerLevel)
{
	if (paramType)
	{
		m_Vector_PointerFunctionParamType = std::move(*paramType);
	}
}

ASTVariableDefine::~ASTVariableDefine()
{
}

std::shared_ptr<HazeCompilerValue> ASTVariableDefine::CodeGen()
{
	struct GlobalVariableInitScope
	{
		GlobalVariableInitScope(HazeCompilerModule* m, HazeSectionSignal signal) : Module(m), Signal(signal)
		{
			if (Signal == HazeSectionSignal::Global)
			{
				Module->PreStartCreateGlobalVariable();
			}
		}

		~GlobalVariableInitScope()
		{
			if (Signal == HazeSectionSignal::Global)
			{
				Module->EndCreateGlobalVariable();
			}
		}
		
		HazeCompilerModule* Module;
		HazeSectionSignal Signal;
	};

	std::shared_ptr<HazeCompilerValue> retValue = nullptr;
	std::unique_ptr<HazeCompilerModule>& currModule = m_Compiler->GetCurrModule();
	GlobalVariableInitScope scope(currModule.get(), m_SectionSignal);

	std::vector<std::shared_ptr<HazeCompilerValue>> sizeValue;
	for (auto& iter : m_ArraySize)
	{
		auto v = iter->CodeGen();
		if (v->IsConstant())
		{
			sizeValue.push_back(v);
		}
		else
		{
			HAZE_LOG_ERR_W("变量<%s>定义失败，定义数组长度必须是常量! 当前函数<%s>\n", m_DefineVariable.Name.c_str(),
				m_SectionSignal == HazeSectionSignal::Local ? currModule->GetCurrFunction()->GetName().c_str() : HAZE_TEXT("None"));
			return nullptr;
		}
	}

	std::shared_ptr<HazeCompilerValue> exprValue = nullptr;
	if (m_Expression)
	{
		if (auto NullExpression = dynamic_cast<ASTNullPtr*>(m_Expression.get()))
		{
			NullExpression->SetDefineType(m_DefineVariable.Type);
		}

		exprValue = m_Expression->CodeGen();
		if (!exprValue)
		{
			return nullptr;
		}
	}

	bool isRef = m_DefineVariable.Type.PrimaryType == HazeValueType::ReferenceBase || m_DefineVariable.Type.PrimaryType == HazeValueType::ReferenceClass;

	if (m_SectionSignal == HazeSectionSignal::Global)
	{
		retValue = m_Compiler->CreateGlobalVariable(currModule, m_DefineVariable, exprValue, sizeValue, &m_Vector_PointerFunctionParamType);
	}
	else if (m_SectionSignal == HazeSectionSignal::Local)
	{
		retValue = m_Compiler->CreateLocalVariable(currModule->GetCurrFunction(), m_DefineVariable, m_Location.Line, exprValue, sizeValue, &m_Vector_PointerFunctionParamType);
		m_Compiler->InsertLineCount(m_Location.Line);
	}
	else if (m_SectionSignal == HazeSectionSignal::Class)
	{
		retValue = m_Compiler->CreateClassVariable(currModule, m_DefineVariable, exprValue, sizeValue, &m_Vector_ClassTemplateType);
	}

	if (retValue && exprValue)
	{
		if (isRef)
		{
			m_Compiler->CreateLea(retValue, exprValue);
		}
		else
		{
			m_Compiler->CreateMov(retValue, exprValue);
		}
	}
	else if (retValue->IsClass())
	{
		auto function = m_Compiler->GetCurrModule()->GetClass(retValue->GetValueType().CustomName)->FindFunction(retValue->GetValueType().CustomName);
		std::vector<std::shared_ptr<HazeCompilerValue>> param;
		m_Compiler->CreateFunctionCall(function, param, retValue);
	}

	return retValue;
}

ASTReturn::ASTReturn(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression)
	:ASTBase(compiler, location), m_Expression(std::move(expression))
{
}

ASTReturn::~ASTReturn()
{
}

std::shared_ptr<HazeCompilerValue> ASTReturn::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	std::shared_ptr<HazeCompilerValue> retValue = m_Expression ? m_Expression->CodeGen() : m_Compiler->GetConstantValueInt(0);

	if (m_Expression ? retValue->GetValueType() == m_Compiler->GetCurrModule()->GetCurrFunction()->GetFunctionType() : 
		IsVoidType(m_Compiler->GetCurrModule()->GetCurrFunction()->GetFunctionType().PrimaryType))
	{
		return m_Compiler->CreateRet(retValue);
	}
	else
	{
		AST_ERR_W("返回值类型错误");
	}

	return nullptr;
}

ASTNew::ASTNew(HazeCompiler* compiler, const SourceLocation& location, const HazeDefineVariable& DefineVar, std::vector<std::unique_ptr<ASTBase>> countArrayExpression)
	: ASTBase(compiler, location, DefineVar), m_CountArrayExpression(std::move(countArrayExpression))
{
}

ASTNew::~ASTNew()
{
}

std::shared_ptr<HazeCompilerValue> ASTNew::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> countValue = nullptr;
	if (m_CountArrayExpression.size() > 0)
	{
		countValue = m_Compiler->CreateMov(m_Compiler->GetTempRegister(), m_Compiler->GetConstantValueUint64(1));

		for (auto& iter : m_CountArrayExpression)
		{
			m_Compiler->CreateMul(countValue, iter->CodeGen());
		}
	}

	auto value = m_Compiler->CreateNew(m_Compiler->GetCurrModule()->GetCurrFunction(), m_DefineVariable.Type, countValue);

	//new申请内存后，若是类的话，需要调用构造函数
	if (value->GetValueType().HasCustomName())
	{
		auto newClass = m_Compiler->GetCurrModule()->GetClass(value->GetValueType().CustomName);
		if (countValue)
		{
			//需要初始化多个类对象
			auto function = newClass->FindFunction(value->GetValueType().CustomName);
			auto arrayConstructorFunc = m_Compiler->GetFunction(HAZE_OBJECT_ARRAY_CONSTRUCTOR);
			std::vector<std::shared_ptr<HazeCompilerValue>> param;

			value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(), value);

			//参数从右往左
			std::vector<std::shared_ptr<HazeCompilerValue>> params;
			params.push_back(countValue);
			params.push_back(m_Compiler->GetConstantValueUint64(newClass->GetDataSize()));
			params.push_back(m_Compiler->CreatePointerToFunction(function));
			params.push_back(value);

			m_Compiler->CreateFunctionCall(arrayConstructorFunc.first, params);
		}
		else
		{
			auto function = newClass->FindFunction(value->GetValueType().CustomName);
			std::vector<std::shared_ptr<HazeCompilerValue>> param;

			value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(), value);
			m_Compiler->CreateFunctionCall(function, param, value);
		}
	}

	return value;
}

ASTGetAddress::ASTGetAddress(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
}

ASTGetAddress::~ASTGetAddress()
{
}

std::shared_ptr<HazeCompilerValue> ASTGetAddress::CodeGen()
{
	std::shared_ptr<HazeCompilerValue> retValue = nullptr;
	if (m_Expression)
	{
		retValue = m_Expression->CodeGen();
	}

	if (!retValue)
	{
		//获得函数地址
		auto function = m_Compiler->GetCurrModule()->GetFunction(m_Expression->GetName());
		if (function.first)
		{
			retValue = m_Compiler->CreatePointerToFunction(function.first);
		}
		else
		{
			HAZE_LOG_ERR_W("未能找到函数<%s>去获得函数地址!\n", m_Expression->GetName());
		}
	}
	else
	{
		retValue = m_Compiler->CreatePointerToValue(retValue);
	}

	return retValue;
}

ASTPointerValue::ASTPointerValue(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression,
	int level, std::unique_ptr<ASTBase> assignExpression)
	: ASTBase(compiler, location), m_Expression(std::move(expression)), m_Level(level), m_AssignExpression(std::move(assignExpression))
{
}

ASTPointerValue::~ASTPointerValue()
{
}

std::shared_ptr<HazeCompilerValue> ASTPointerValue::CodeGen()
{
	auto pointerValue = m_Expression->CodeGen();

	if (pointerValue)
	{
		if (m_AssignExpression)
		{
			return m_Compiler->CreateMovToPV(pointerValue, m_AssignExpression->CodeGen());
		}

		return m_Compiler->CreateMovPV(m_Compiler->GetTempRegister(), pointerValue);
	}
	else
	{
		HAZE_LOG_ERR_W("未能获得<%s>指针指向的值!\n", m_DefineVariable.Name.c_str());
	}

	return nullptr;
}

ASTNeg::ASTNeg(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression, bool isNumberNeg)
	: ASTBase(compiler, location), m_Expression(std::move(expression)), m_IsNumberNeg(isNumberNeg)
{
}

ASTNeg::~ASTNeg()
{
}

std::shared_ptr<HazeCompilerValue> ASTNeg::CodeGen()
{
	if (m_IsNumberNeg)
	{
		return m_Compiler->CreateNeg(m_Expression->CodeGen());
	}
	else
	{
		return m_Compiler->CreateBitNeg(m_Expression->CodeGen());
	}
}

ASTNullPtr::ASTNullPtr(HazeCompiler* compiler, const SourceLocation& location) : ASTBase(compiler, location)
{
}

ASTNullPtr::~ASTNullPtr()
{
}

std::shared_ptr<HazeCompilerValue> ASTNullPtr::CodeGen()
{
	return m_Compiler->GetNullPtr(m_DefineVariable.Type);
}

void ASTNullPtr::SetDefineType(const HazeDefineType& type)
{
	m_DefineVariable.Type = type;
}

ASTNot::ASTNot(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
}

ASTNot::~ASTNot()
{
}

std::shared_ptr<HazeCompilerValue> ASTNot::CodeGen()
{
	return m_Compiler->CreateNot(m_Compiler->GetTempRegister(), m_Expression->CodeGen());
}


ASTInc::ASTInc(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression, bool isPreInc)
	: ASTBase(compiler, location), m_IsPreInc(isPreInc), m_Expression(std::move(expression))
{
}

ASTInc::~ASTInc()
{
}

std::shared_ptr<HazeCompilerValue> ASTInc::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	return m_Compiler->CreateInc(m_Expression->CodeGen(), m_IsPreInc);
}

ASTDec::ASTDec(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression, bool isPreDec)
	: ASTBase(compiler, location), m_IsPreDec(isPreDec), m_Expression(std::move(expression))
{
}

ASTDec::~ASTDec()
{
}

std::shared_ptr<HazeCompilerValue> ASTDec::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	return m_Compiler->CreateDec(m_Expression->CodeGen(), m_IsPreDec);
}

ASTOperetorAssign::ASTOperetorAssign(HazeCompiler* compiler, const SourceLocation& location, HazeToken token, 
	std::unique_ptr<ASTBase>& expression) 
	: ASTBase(compiler, location), m_Token(token), m_Expression(std::move(expression))
{
}

ASTOperetorAssign::~ASTOperetorAssign()
{
}

std::shared_ptr<HazeCompilerValue> ASTOperetorAssign::CodeGen()
{
	return m_Expression->CodeGen();
}

ASTMultiExpression::ASTMultiExpression(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	std::vector<std::unique_ptr<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(std::move(expressions))
{
}

ASTMultiExpression::~ASTMultiExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTMultiExpression::CodeGen()
{
	for (size_t i = 0; i < m_Expressions.size(); i++)
	{
		m_Expressions[i]->CodeGen();
	}

	return nullptr;
}

ASTBinaryExpression::ASTBinaryExpression(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	HazeToken operatorToken, std::unique_ptr<ASTBase>& leftAST, std::unique_ptr<ASTBase>& rightAST)
	:ASTBase(compiler, location), m_SectionSignal(section), m_OperatorToken(operatorToken), m_LeftAST(std::move(leftAST)),
	m_RightAST(std::move(rightAST)), m_LeftBlock(nullptr), m_RightBlock(nullptr), m_DefaultBlock(nullptr)
{
}

ASTBinaryExpression::~ASTBinaryExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTBinaryExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto rightExp = dynamic_cast<ASTBinaryExpression*>(m_RightAST.get());
	if (rightExp && IsAndOrToken(rightExp->m_OperatorToken))
	{
		rightExp->SetLeftAndRightBlock(m_LeftBlock, m_RightBlock);
	}

	auto leftExp = dynamic_cast<ASTBinaryExpression*>(m_LeftAST.get());
	if (leftExp && IsAndOrToken(leftExp->m_OperatorToken))
	{
		leftExp->SetLeftAndRightBlock(m_LeftBlock, m_RightBlock);
	}

	switch (m_OperatorToken)
	{
	case HazeToken::Add:
		return m_Compiler->CreateAdd(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::Sub:
		return m_Compiler->CreateSub(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::Mul:
		return m_Compiler->CreateMul(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::Div:
		return m_Compiler->CreateDiv(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::Inc:
		return m_Compiler->CreateInc(m_LeftAST->CodeGen(), false);
	case HazeToken::Dec:
		return m_Compiler->CreateDec(m_LeftAST->CodeGen(), false);
	case HazeToken::Mod:
		return m_Compiler->CreateMod(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::Not:
		return m_Compiler->CreateNot(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::Shl:
		return m_Compiler->CreateShl(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::Shr:
		return m_Compiler->CreateShr(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::Assign:
		return m_Compiler->CreateMov(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::BitAnd:
		return m_Compiler->CreateBitAnd(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::BitOr:
		return m_Compiler->CreateBitOr(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::BitXor:
		return m_Compiler->CreateBitXor(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
	case HazeToken::AddAssign:
		return m_Compiler->CreateAdd(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::SubAssign:
		return m_Compiler->CreateSub(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::MulAssign:
		return m_Compiler->CreateMul(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::DivAssign:
		return m_Compiler->CreateDiv(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::ModAssign:
		return m_Compiler->CreateMod(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::BitAndAssign:
		return m_Compiler->CreateBitAnd(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::BitOrAssign:
		return m_Compiler->CreateBitOr(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::BitXorAssign:
		return m_Compiler->CreateBitXor(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::ShlAssign:
		return m_Compiler->CreateShl(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::ShrAssign:
		return m_Compiler->CreateShr(m_LeftAST->CodeGen(), m_RightAST->CodeGen(), true);
	case HazeToken::And:
	{
		auto leftValue = m_LeftAST->CodeGen();
		if (!leftExp)
		{
			m_Compiler->CreateBoolCmp(m_LeftAST->CodeGen());
		}
		else
		{
			m_Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(leftExp->m_OperatorToken), m_RightAST ? nullptr : m_LeftBlock ? m_LeftBlock->GetShared() : nullptr,
				m_DefaultBlock ? m_DefaultBlock->GetShared() : m_RightBlock->GetShared());
		}

		if (rightExp)
		{
			m_RightAST->CodeGen();
			if (!dynamic_cast<ASTBinaryExpression*>(rightExp->m_RightAST.get()))
			{
				m_Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(rightExp->m_OperatorToken), m_LeftBlock ? m_LeftBlock->GetShared() : nullptr,
					m_DefaultBlock ? m_DefaultBlock->GetShared() : m_RightBlock->GetShared());
			}
		}
		else
		{
			if (!leftExp)
			{
				m_Compiler->CreateCompareJmp(leftExp ? GetHazeCmpTypeByToken(leftExp->m_OperatorToken) : HazeCmpType::Equal,
					nullptr, m_RightBlock->GetShared());
			}

			auto rightValue = m_RightAST->CodeGen();
			m_Compiler->CreateBoolCmp(rightValue);
			m_Compiler->CreateCompareJmp(HazeCmpType::Equal, m_LeftBlock ? m_LeftBlock->GetShared() : nullptr,
				m_DefaultBlock ? m_DefaultBlock->GetShared() : m_RightBlock->GetShared());
		}

		return leftValue;
	}
	case HazeToken::Or:
	{
		auto leftValue = m_LeftAST->CodeGen();
		/*if (m_DefaultBlock)
		{
			m_Compiler->SetInsertBlock(m_DefaultBlock->GetShared());
		}
		else*/
		{
			if (!leftExp)
			{
				m_Compiler->CreateBoolCmp(leftValue);
			}
			else
			{
				m_Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(leftExp->m_OperatorToken), m_LeftBlock ? m_LeftBlock->GetShared() : nullptr, nullptr);
			}

			if (rightExp)
			{
				m_RightAST->CodeGen();
				if (!dynamic_cast<ASTBinaryExpression*>(rightExp->m_RightAST.get()))
				{
					m_Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(rightExp->m_OperatorToken), m_LeftBlock ? m_LeftBlock->GetShared() : nullptr,
						m_DefaultBlock ? m_DefaultBlock->GetShared() : m_RightBlock->GetShared());
				}
			}
			else
			{
				if (!leftExp)
				{
					m_Compiler->CreateCompareJmp(leftExp ? GetHazeCmpTypeByToken(leftExp->m_OperatorToken) : HazeCmpType::Equal,
						m_LeftBlock ? m_LeftBlock->GetShared() : nullptr, nullptr);
				}

				auto rightValue = m_RightAST->CodeGen();
				m_Compiler->CreateBoolCmp(rightValue);
				m_Compiler->CreateCompareJmp(HazeCmpType::Equal, m_LeftBlock ? m_LeftBlock->GetShared() : nullptr,
					m_DefaultBlock ? m_DefaultBlock->GetShared() : m_RightBlock->GetShared());
			}
		}

		return leftValue;
	}
	case HazeToken::Equal:
	case HazeToken::NotEqual:
	case HazeToken::Greater:
	case HazeToken::GreaterEqual:
	case HazeToken::Less:
	case HazeToken::LessEqual:
	{
		auto retValue = m_Compiler->CreateIntCmp(m_LeftAST->CodeGen(), m_RightAST->CodeGen());
		if (m_LeftBlock || m_RightBlock)
		{
			m_Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(m_OperatorToken), m_LeftBlock ? m_LeftBlock->GetShared() : nullptr,
				m_RightBlock ? m_RightBlock->GetShared() : nullptr);
		}

		return retValue;
	}
	
	default:
		break;
	}

	return nullptr;
}

void ASTBinaryExpression::SetLeftAndRightBlock(HazeBaseBlock* leftJmpBlock, HazeBaseBlock* rightJmpBlock)
{
	m_LeftBlock = leftJmpBlock;
	m_RightBlock = rightJmpBlock;
}

ASTThreeExpression::ASTThreeExpression(HazeCompiler* compiler, const SourceLocation& location
	, std::unique_ptr<ASTBase>& conditionAST, std::unique_ptr<ASTBase>& leftAST, std::unique_ptr<ASTBase>& rightAST)
	: ASTBase(compiler, location), m_ConditionAST(std::move(conditionAST)), m_LeftAST(std::move(leftAST)), 
		m_RightAST(std::move(rightAST))
{
}

ASTThreeExpression::~ASTThreeExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTThreeExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();

	auto parentBlock = m_Compiler->GetInsertBlock();

	auto defauleBlock = HazeBaseBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
	auto blockLeft = HazeBaseBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
	auto blockRight = HazeBaseBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);

	auto conditionExp = dynamic_cast<ASTBinaryExpression*>(m_ConditionAST.get());
	if (conditionExp)
	{
		conditionExp->CodeGen();
		m_Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(conditionExp->m_OperatorToken), blockLeft, blockRight);
	}
	else
	{
		m_Compiler->CreateBoolCmp(m_ConditionAST->CodeGen());
		m_Compiler->CreateCompareJmp(HazeCmpType::Equal, blockLeft, blockRight);
	}

	auto tempValue = HazeCompiler::GetTempRegister();
	m_Compiler->SetInsertBlock(blockLeft);
	m_Compiler->CreateMov(tempValue, m_LeftAST->CodeGen());
	m_Compiler->CreateJmpToBlock(defauleBlock);

	m_Compiler->SetInsertBlock(blockRight);
	m_Compiler->CreateMov(tempValue, m_RightAST->CodeGen());
	m_Compiler->CreateJmpToBlock(defauleBlock);

	m_Compiler->SetInsertBlock(defauleBlock);

	return tempValue;
}

ASTImportModule::ASTImportModule(HazeCompiler* compiler, const SourceLocation& location, const HAZE_STRING& moduleName)
	: ASTBase(compiler, location), m_ModuleName(moduleName)
{
}

ASTImportModule::~ASTImportModule()
{
}

std::shared_ptr<HazeCompilerValue> ASTImportModule::CodeGen()
{
	m_Compiler->AddImportModuleToCurrModule(m_Compiler->ParseModule(m_ModuleName));
	return nullptr;
}

ASTBreakExpression::ASTBreakExpression(HazeCompiler* compiler, const SourceLocation& location) 
	: ASTBase(compiler, location)
{
}

ASTBreakExpression::~ASTBreakExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTBreakExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	m_Compiler->CreateJmpToBlock(m_Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopEnd()->GetShared());
	return nullptr;
}

ASTContinueExpression::ASTContinueExpression(HazeCompiler* compiler, const SourceLocation& location)
	: ASTBase(compiler, location)
{
}

ASTContinueExpression::~ASTContinueExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTContinueExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	m_Compiler->CreateJmpToBlock(m_Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopStep()->GetShared());
	return nullptr;
}

ASTIfExpression::ASTIfExpression(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& condition,
	std::unique_ptr<ASTBase>& ifExpression, std::unique_ptr<ASTBase>& elseExpression)
	: ASTBase(compiler, location),m_Condition(std::move(condition)), m_IfExpression(std::move(ifExpression)),
		m_ElseExpression(std::move(elseExpression))
{
}

ASTIfExpression::~ASTIfExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTIfExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();

	auto parentBlock = m_Compiler->GetInsertBlock();
	auto ifThenBlock = HazeBaseBlock::CreateBaseBlock(function->GenIfThenBlockName(), function, parentBlock);
	auto elseBlock = m_ElseExpression ? HazeBaseBlock::CreateBaseBlock(function->GenElseBlockName(), function, parentBlock) : nullptr;
	auto nextBlock = HazeBaseBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);

	auto ConditionExp = dynamic_cast<ASTBinaryExpression*>(m_Condition.get());

	if (ConditionExp && GetHazeCmpTypeByToken(ConditionExp->m_OperatorToken) != HazeCmpType::None)
	{
		ConditionExp->SetLeftAndRightBlock(ifThenBlock.get(), m_ElseExpression ? elseBlock.get() : nextBlock.get());
		m_Condition->CodeGen();
	}
	else
	{
		m_Compiler->CreateBoolCmp(m_Condition->CodeGen());
		m_Compiler->CreateCompareJmp(HazeCmpType::Equal, ifThenBlock, m_ElseExpression ? elseBlock : nextBlock);
	}

	m_Compiler->SetInsertBlock(ifThenBlock);
	m_IfExpression->CodeGen();
	m_Compiler->CreateJmpToBlock(nextBlock);

	if (m_ElseExpression)
	{
		m_Compiler->SetInsertBlock(elseBlock);
		m_ElseExpression->CodeGen();
		m_Compiler->CreateJmpToBlock(nextBlock);
	}

	m_Compiler->SetInsertBlock(nextBlock);
	return nullptr;
}

bool ASTIfExpression::HasElseExpression() const
{
	auto ifExp = dynamic_cast<ASTIfExpression*>(m_ElseExpression.get());
	if (ifExp)
	{
		return ifExp->HasElseExpression();
	}
	else
	{
		return m_ElseExpression != nullptr;
	}
}

ASTWhileExpression::ASTWhileExpression(HazeCompiler* compiler, const SourceLocation& location, 
	std::unique_ptr<ASTBase>& condition, std::unique_ptr<ASTBase>& multiExpression)
	:ASTBase(compiler, location), m_Condition(std::move(condition)), m_MultiExpression(std::move(multiExpression))
{
}

ASTWhileExpression::~ASTWhileExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTWhileExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();

	auto parentBlock = m_Compiler->GetInsertBlock();
	auto whileBlock = HazeBaseBlock::CreateBaseBlock(function->GenWhileBlockName(), function, parentBlock);
	auto nextBlock = HazeBaseBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);

	whileBlock->SetLoopEnd(nextBlock.get());
	whileBlock->SetLoopStep(whileBlock.get());

	auto conditionExp = dynamic_cast<ASTBinaryExpression*>(m_Condition.get());
	if (conditionExp)
	{
		conditionExp->SetLeftAndRightBlock(whileBlock.get(), nextBlock.get());
		m_Condition->CodeGen();
	}
	else
	{
		m_Compiler->CreateBoolCmp(m_Condition->CodeGen());
		m_Compiler->CreateCompareJmp(HazeCmpType::Equal, whileBlock, nextBlock);
	}

	m_Compiler->SetInsertBlock(whileBlock);
	m_MultiExpression->CodeGen();
	m_Compiler->CreateJmpToBlock(whileBlock);

	m_Compiler->SetInsertBlock(nextBlock);
	return nullptr;
}

ASTForExpression::ASTForExpression(HazeCompiler* compiler, const SourceLocation& location,
	std::unique_ptr<ASTBase>& initExpression, std::unique_ptr<ASTBase>& conditionExpression, 
	std::unique_ptr<ASTBase>& stepExpression, std::unique_ptr<ASTBase>& multiExpression)
	: ASTBase(compiler, location), m_InitExpression(std::move(initExpression)), m_ConditionExpression(std::move(conditionExpression)),
		m_StepExpression(std::move(stepExpression)), m_MultiExpression(std::move(multiExpression))
{
}

ASTForExpression::~ASTForExpression()
{
}

std::shared_ptr<HazeCompilerValue> ASTForExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();
	auto parentBlock = m_Compiler->GetInsertBlock();

	auto forConditionBlock = HazeBaseBlock::CreateBaseBlock(function->GenForConditionBlockName(), function, parentBlock);
	auto loopBlock = HazeBaseBlock::CreateBaseBlock(function->GenLoopBlockName(), function, parentBlock);
	auto forStepBlock = HazeBaseBlock::CreateBaseBlock(function->GenForStepBlockName(), function, parentBlock);
	auto endLoopBlock = HazeBaseBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
	loopBlock->SetLoopEnd(endLoopBlock.get());
	loopBlock->SetLoopStep(forStepBlock.get());

	if (!m_InitExpression->CodeGen())
	{
		HAZE_LOG_ERR_W("循环语句初始化失败!\n");
		return nullptr;
	}

	m_Compiler->CreateJmpToBlock(forConditionBlock);
	m_Compiler->SetInsertBlock(forConditionBlock);
	auto conditionExp = dynamic_cast<ASTBinaryExpression*>(m_ConditionExpression.get());
	if (conditionExp)
	{
		conditionExp->SetLeftAndRightBlock(loopBlock.get(), endLoopBlock.get());
		m_ConditionExpression->CodeGen();
	}
	else
	{
		m_Compiler->CreateBoolCmp(m_ConditionExpression->CodeGen());
		m_Compiler->CreateCompareJmp(HazeCmpType::Equal, loopBlock, endLoopBlock);
	}

	m_Compiler->SetInsertBlock(loopBlock);
	m_MultiExpression->CodeGen();
	m_Compiler->CreateJmpToBlock(forStepBlock);

	m_Compiler->SetInsertBlock(forStepBlock);
	m_StepExpression->CodeGen();
	m_Compiler->CreateJmpToBlock(forConditionBlock);

	m_Compiler->SetInsertBlock(endLoopBlock);

	return nullptr;
}

ASTInitializeList::ASTInitializeList(HazeCompiler* compiler, const SourceLocation& location, 
	std::vector<std::unique_ptr<ASTBase>>& initializeListExpression)
	: ASTBase(compiler, location), m_InitializeListExpression(std::move(initializeListExpression))
{
}

ASTInitializeList::~ASTInitializeList()
{
}

std::shared_ptr<HazeCompilerValue> ASTInitializeList::CodeGen()
{
	std::vector<std::shared_ptr<HazeCompilerValue>> values;
	for (size_t i = 0; i < m_InitializeListExpression.size(); i++)
	{
		values.push_back(m_InitializeListExpression[i]->CodeGen());
	}

	auto InitilaizeListValue = HazeCompiler::GetInitializeListValue();
	InitilaizeListValue->ResetInitializeList(values);

	return InitilaizeListValue;
}

ASTCast::ASTCast(HazeCompiler* compiler, const SourceLocation& location, HazeDefineType& type, std::unique_ptr<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
	m_DefineVariable.Type = type;
}

ASTCast::~ASTCast()
{
}

std::shared_ptr<HazeCompilerValue> ASTCast::CodeGen()
{
	return m_Compiler->CreateCast(m_DefineVariable.Type, m_Expression->CodeGen());
}

ASTArrayLength::ASTArrayLength(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
}

ASTArrayLength::~ASTArrayLength()
{
}

std::shared_ptr<HazeCompilerValue> ASTArrayLength::CodeGen()
{
	return m_Compiler->CreateGetArrayLength(m_Expression->CodeGen());
}

ASTSizeOf::ASTSizeOf(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
}

ASTSizeOf::~ASTSizeOf()
{
}

std::shared_ptr<HazeCompilerValue> ASTSizeOf::CodeGen()
{
	return m_Compiler->GetConstantValueUint64(m_Expression->CodeGen()->GetSize());
}
