#include "HazePch.h"
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
#include "HazeCompilerEnum.h"
#include "HazeCompilerEnumValue.h"
#include "HazeCompilerHelper.h"

//Base
ASTBase::ASTBase(HazeCompiler* compiler, const SourceLocation& location) : m_Compiler(compiler), m_Location(location)
{
	m_DefineVariable.Name = H_TEXT("");
	m_DefineVariable.Type = { HazeValueType::Void, H_TEXT("") };
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

Share<HazeCompilerValue> ASTBool::CodeGen()
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

Share<HazeCompilerValue> ASTNumber::CodeGen()
{
	return m_Compiler->GenConstantValue(m_DefineVariable.Type.PrimaryType, m_Value);
}

ASTStringText::ASTStringText(HazeCompiler* compiler, const SourceLocation& location, HString& text) 
	: ASTBase(compiler, location), m_Text(std::move(text))
{
}

ASTStringText::~ASTStringText()
{
}

Share<HazeCompilerValue> ASTStringText::CodeGen()
{
	return m_Compiler->GenStringVariable(m_Text);
}

ASTIdentifier::ASTIdentifier(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, HString& name,
	V_Array<Unique<ASTBase>>& arrayIndexExpression, HString nameSpace)
	: ASTBase(compiler, location), m_SectionSignal(section), m_ArrayIndexExpression(Move(arrayIndexExpression)),
	m_NameSpace(Move(nameSpace))
{
	m_DefineVariable.Name = Move(name);
}

ASTIdentifier::~ASTIdentifier()
{
}

Share<HazeCompilerValue> ASTIdentifier::CodeGen()
{
	Share<HazeCompilerValue> retValue = nullptr;
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
	else if (m_SectionSignal == HazeSectionSignal::Enum)
	{
		retValue = m_Compiler->GetEnumVariable(m_Compiler->GetCurrModule()->GetCurrEnumName(), m_DefineVariable.Name);
		if (m_Compiler->GetCurrModule()->GetCurrEnum())
		{
			retValue = m_Compiler->GenConstantValue(m_Compiler->GetCurrModule()->GetCurrEnum()->GetParentType(), retValue->GetValue());
		}
	}

	if (!m_NameSpace.empty())
	{
		retValue = m_Compiler->GetEnumVariable(m_NameSpace, m_DefineVariable.Name);
	}

	if (retValue)
	{
		if (retValue->IsClassMember())
		{
			retValue->SetScope(classMemberScope);
		}

		if (retValue->IsArray() && m_ArrayIndexExpression.size() > 0)
		{
			V_Array<Share<HazeCompilerValue>> indexValue;
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
				m_SectionSignal == HazeSectionSignal::Local ? m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str() : H_TEXT("None"));
			return nullptr;
		}
	}

	return retValue;
}

ASTFunctionCall::ASTFunctionCall(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	HString& name, V_Array<Unique<ASTBase>>& functionParam)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Name(std::move(name)), m_FunctionParam(std::move(functionParam))
{
}

ASTFunctionCall::~ASTFunctionCall()
{
}

Share<HazeCompilerValue> ASTFunctionCall::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto [function, objectVariable] = m_Compiler->GetFunction(m_Name);

	V_Array<Share<HazeCompilerValue>> param;

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
	const HazeDefineVariable& defineVar, Unique<ASTBase> expression, V_Array<Unique<ASTBase>> arraySizeOrParams,
	int pointerLevel, V_Array<HazeDefineType>* paramType)
	: ASTBase(compiler, location, defineVar), m_SectionSignal(section), m_Expression(std::move(expression)), 
		m_ArraySizeOrParams(std::move(arraySizeOrParams)), m_PointerLevel(pointerLevel)
{
	if (paramType)
	{
		m_Vector_PointerFunctionParamType = std::move(*paramType);
	}
}

ASTVariableDefine::~ASTVariableDefine()
{
}

Share<HazeCompilerValue> ASTVariableDefine::CodeGen()
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

	Share<HazeCompilerValue> retValue = nullptr;
	Unique<HazeCompilerModule>& currModule = m_Compiler->GetCurrModule();
	GlobalVariableInitScope scope(currModule.get(), m_SectionSignal);

	V_Array<Share<HazeCompilerValue>> sizeValue;
	
	if (IsArrayType(m_DefineVariable.Type.PrimaryType))
	{
		for (auto& iter : m_ArraySizeOrParams)
		{
			auto v = iter->CodeGen();
			if (v->IsConstant())
			{
				sizeValue.push_back(v);
			}
			else
			{
				HAZE_LOG_ERR_W("变量<%s>定义失败，定义数组长度必须是常量! 当前函数<%s>\n", m_DefineVariable.Name.c_str(),
					m_SectionSignal == HazeSectionSignal::Local ? currModule->GetCurrFunction()->GetName().c_str() : H_TEXT("None"));
				return nullptr;
			}
		}
	}
	else if (!IsClassType(m_DefineVariable.Type.PrimaryType) && m_ArraySizeOrParams.size() > 0)
	{
		HAZE_LOG_ERR_W("变量<%s>定义失败，非数组变量传入数组变量! 当前函数<%s>\n", m_DefineVariable.Name.c_str(),
			m_SectionSignal == HazeSectionSignal::Local ? currModule->GetCurrFunction()->GetName().c_str() : H_TEXT("None"));
	}
	

	Share<HazeCompilerValue> exprValue = nullptr;
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
		V_Array<Share<HazeCompilerValue>> param(m_ArraySizeOrParams.size());

		//构造参数
		for (int i = 0; i < m_ArraySizeOrParams.size(); i++)
		{
			param[i] = m_ArraySizeOrParams[m_ArraySizeOrParams.size() - 1 - i]->CodeGen();
		}

		m_Compiler->CreateFunctionCall(function, param, retValue);
	}

	return retValue;
}

ASTReturn::ASTReturn(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	:ASTBase(compiler, location), m_Expression(std::move(expression))
{
}

ASTReturn::~ASTReturn()
{
}

Share<HazeCompilerValue> ASTReturn::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	Share<HazeCompilerValue> retValue = m_Expression ? m_Expression->CodeGen() : m_Compiler->GetConstantValueInt(0);

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

ASTNew::ASTNew(HazeCompiler* compiler, const SourceLocation& location, const HazeDefineVariable& DefineVar, V_Array<Unique<ASTBase>> countArrayExpression
	, V_Array<Unique<ASTBase>> constructorParam)
	: ASTBase(compiler, location, DefineVar), m_CountArrayExpression(std::move(countArrayExpression)), m_ConstructorParam(std::move(constructorParam))
{
}

ASTNew::~ASTNew()
{
}

Share<HazeCompilerValue> ASTNew::CodeGen()
{
	Share<HazeCompilerValue> countValue = nullptr;
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
			//需要初始化多个类对象，类的参数个数必须为0
			auto function = newClass->FindFunction(value->GetValueType().CustomName);
			auto arrayConstructorFunc = m_Compiler->GetFunction(HAZE_OBJECT_ARRAY_CONSTRUCTOR);
			V_Array<Share<HazeCompilerValue>> param;

			value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(), value);

			if (function->HasParam())
			{
				HAZE_LOG_ERR_W("生成类对象数组的构造函数必须是无参数的!\n");
				return value;
			}

			//参数从右往左
			V_Array<Share<HazeCompilerValue>> params;
			params.push_back(countValue);
			params.push_back(m_Compiler->GetConstantValueUint64(newClass->GetDataSize()));
			params.push_back(m_Compiler->CreatePointerToFunction(function));
			params.push_back(value);

			m_Compiler->CreateFunctionCall(arrayConstructorFunc.first, params);
		}
		else
		{
			auto function = newClass->FindFunction(value->GetValueType().CustomName);
			V_Array<Share<HazeCompilerValue>> param;

			//构造参数
			for (auto& p : m_ConstructorParam)
			{
				param.push_back(p->CodeGen());
			}

			value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(), value);
			m_Compiler->CreateFunctionCall(function, param, value);
		}
	}

	return value;
}

ASTGetAddress::ASTGetAddress(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
}

ASTGetAddress::~ASTGetAddress()
{
}

Share<HazeCompilerValue> ASTGetAddress::CodeGen()
{
	Share<HazeCompilerValue> retValue = nullptr;
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

ASTPointerValue::ASTPointerValue(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression,
	int level, Unique<ASTBase> assignExpression)
	: ASTBase(compiler, location), m_Expression(std::move(expression)), m_Level(level), m_AssignExpression(std::move(assignExpression))
{
}

ASTPointerValue::~ASTPointerValue()
{
}

Share<HazeCompilerValue> ASTPointerValue::CodeGen()
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

ASTNeg::ASTNeg(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isNumberNeg)
	: ASTBase(compiler, location), m_Expression(std::move(expression)), m_IsNumberNeg(isNumberNeg)
{
}

ASTNeg::~ASTNeg()
{
}

Share<HazeCompilerValue> ASTNeg::CodeGen()
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

Share<HazeCompilerValue> ASTNullPtr::CodeGen()
{
	return m_Compiler->GetNullPtr(m_DefineVariable.Type);
}

void ASTNullPtr::SetDefineType(const HazeDefineType& type)
{
	m_DefineVariable.Type = type;
}

ASTNot::ASTNot(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
}

ASTNot::~ASTNot()
{
}

Share<HazeCompilerValue> ASTNot::CodeGen()
{
	return m_Compiler->CreateNot(m_Compiler->GetTempRegister(), m_Expression->CodeGen());
}


ASTInc::ASTInc(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreInc)
	: ASTBase(compiler, location), m_IsPreInc(isPreInc), m_Expression(std::move(expression))
{
}

ASTInc::~ASTInc()
{
}

Share<HazeCompilerValue> ASTInc::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	return m_Compiler->CreateInc(m_Expression->CodeGen(), m_IsPreInc);
}

ASTDec::ASTDec(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreDec)
	: ASTBase(compiler, location), m_IsPreDec(isPreDec), m_Expression(std::move(expression))
{
}

ASTDec::~ASTDec()
{
}

Share<HazeCompilerValue> ASTDec::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	return m_Compiler->CreateDec(m_Expression->CodeGen(), m_IsPreDec);
}

ASTOperetorAssign::ASTOperetorAssign(HazeCompiler* compiler, const SourceLocation& location, HazeToken token, 
	Unique<ASTBase>& expression) 
	: ASTBase(compiler, location), m_Token(token), m_Expression(std::move(expression))
{
}

ASTOperetorAssign::~ASTOperetorAssign()
{
}

Share<HazeCompilerValue> ASTOperetorAssign::CodeGen()
{
	return m_Expression->CodeGen();
}

ASTMultiExpression::ASTMultiExpression(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	V_Array<Unique<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(std::move(expressions))
{
}

ASTMultiExpression::~ASTMultiExpression()
{
}

Share<HazeCompilerValue> ASTMultiExpression::CodeGen()
{
	for (size_t i = 0; i < m_Expressions.size(); i++)
	{
		m_Expressions[i]->CodeGen();
	}

	return nullptr;
}

ASTBinaryExpression::ASTBinaryExpression(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	HazeToken operatorToken, Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST)
	:ASTBase(compiler, location), m_SectionSignal(section), m_OperatorToken(operatorToken), m_LeftAST(std::move(leftAST)),
	m_RightAST(std::move(rightAST)), m_LeftBlock(nullptr), m_RightBlock(nullptr), m_DefaultBlock(nullptr)
{
}

ASTBinaryExpression::~ASTBinaryExpression()
{
}

Share<HazeCompilerValue> ASTBinaryExpression::CodeGen()
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
	, Unique<ASTBase>& conditionAST, Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST)
	: ASTBase(compiler, location), m_ConditionAST(std::move(conditionAST)), m_LeftAST(std::move(leftAST)), 
		m_RightAST(std::move(rightAST))
{
}

ASTThreeExpression::~ASTThreeExpression()
{
}

Share<HazeCompilerValue> ASTThreeExpression::CodeGen()
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

ASTImportModule::ASTImportModule(HazeCompiler* compiler, const SourceLocation& location, const HString& modulepath)
	: ASTBase(compiler, location), m_ModulePath(modulepath)
{
}

ASTImportModule::~ASTImportModule()
{
}

Share<HazeCompilerValue> ASTImportModule::CodeGen()
{
	m_Compiler->AddImportModuleToCurrModule(m_Compiler->ParseModule(m_ModulePath));
	return nullptr;
}

ASTBreakExpression::ASTBreakExpression(HazeCompiler* compiler, const SourceLocation& location) 
	: ASTBase(compiler, location)
{
}

ASTBreakExpression::~ASTBreakExpression()
{
}

Share<HazeCompilerValue> ASTBreakExpression::CodeGen()
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

Share<HazeCompilerValue> ASTContinueExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	m_Compiler->CreateJmpToBlock(m_Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopStep()->GetShared());
	return nullptr;
}

ASTIfExpression::ASTIfExpression(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& condition,
	Unique<ASTBase>& ifExpression, Unique<ASTBase>& elseExpression)
	: ASTBase(compiler, location),m_Condition(std::move(condition)), m_IfExpression(std::move(ifExpression)),
		m_ElseExpression(std::move(elseExpression))
{
}

ASTIfExpression::~ASTIfExpression()
{
}

Share<HazeCompilerValue> ASTIfExpression::CodeGen()
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
	Unique<ASTBase>& condition, Unique<ASTBase>& multiExpression)
	:ASTBase(compiler, location), m_Condition(std::move(condition)), m_MultiExpression(std::move(multiExpression))
{
}

ASTWhileExpression::~ASTWhileExpression()
{
}

Share<HazeCompilerValue> ASTWhileExpression::CodeGen()
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
	Unique<ASTBase>& initExpression, Unique<ASTBase>& conditionExpression, 
	Unique<ASTBase>& stepExpression, Unique<ASTBase>& multiExpression)
	: ASTBase(compiler, location), m_InitExpression(std::move(initExpression)), m_ConditionExpression(std::move(conditionExpression)),
		m_StepExpression(std::move(stepExpression)), m_MultiExpression(std::move(multiExpression))
{
}

ASTForExpression::~ASTForExpression()
{
}

Share<HazeCompilerValue> ASTForExpression::CodeGen()
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
	V_Array<Unique<ASTBase>>& initializeListExpression)
	: ASTBase(compiler, location), m_InitializeListExpression(std::move(initializeListExpression))
{
}

ASTInitializeList::~ASTInitializeList()
{
}

Share<HazeCompilerValue> ASTInitializeList::CodeGen()
{
	V_Array<Share<HazeCompilerValue>> values;
	for (size_t i = 0; i < m_InitializeListExpression.size(); i++)
	{
		values.push_back(m_InitializeListExpression[i]->CodeGen());
	}

	auto InitilaizeListValue = HazeCompiler::GetInitializeListValue();
	InitilaizeListValue->ResetInitializeList(values);

	return InitilaizeListValue;
}

ASTCast::ASTCast(HazeCompiler* compiler, const SourceLocation& location, HazeDefineType& type, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
	m_DefineVariable.Type = type;
}

ASTCast::~ASTCast()
{
}

Share<HazeCompilerValue> ASTCast::CodeGen()
{
	return m_Compiler->CreateCast(m_DefineVariable.Type, m_Expression->CodeGen());
}

ASTArrayLength::ASTArrayLength(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
}

ASTArrayLength::~ASTArrayLength()
{
}

Share<HazeCompilerValue> ASTArrayLength::CodeGen()
{
	return m_Compiler->CreateGetArrayLength(m_Expression->CodeGen());
}

ASTSizeOf::ASTSizeOf(HazeCompiler* compiler, const SourceLocation& location, const HazeDefineType& type, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(std::move(expression))
{
	m_DefineVariable.Type = type;
}

ASTSizeOf::~ASTSizeOf()
{
}

Share<HazeCompilerValue> ASTSizeOf::CodeGen()
{
	if (m_Expression)
	{
		return m_Compiler->GetConstantValueUint64(m_Expression->CodeGen()->GetSize());
	}
	else
	{
		return m_Compiler->GetConstantValueUint64(GetSizeByType(m_DefineVariable.Type, m_Compiler->GetCurrModule().get()));
	}
}
