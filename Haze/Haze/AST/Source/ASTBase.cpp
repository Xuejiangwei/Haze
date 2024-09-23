#include "HazePch.h"
#include "ASTBase.h"
#include "HazeLogDefine.h"

#include "CompilerHelper.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerClass.h"
#include "CompilerBlock.h"
#include "CompilerFunction.h"
#include "CompilerValue.h"
#include "CompilerElementValue.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerClassValue.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"
#include "CompilerHelper.h"

//Base
ASTBase::ASTBase(Compiler* compiler, const SourceLocation& location) : m_Compiler(compiler), m_Location(location)
{
	m_Value.Value.UInt64 = 0;
	m_DefineVariable.Name = H_TEXT("");
	m_DefineVariable.Type = { HazeValueType::Void };
}

ASTBase::ASTBase(Compiler* compiler, const SourceLocation& location, const HazeDefineVariable& var) 
	: m_Compiler(compiler), m_DefineVariable(var), m_Location(location)
{
	memset(&m_Value.Value, 0, sizeof(m_Value.Value));
}

ASTBool::ASTBool(Compiler* compiler, const SourceLocation& location, const HazeValue& value) : ASTBase(compiler, location)
{
	m_DefineVariable.Type.PrimaryType = HazeValueType::Bool;
	m_Value = value;
}

Share<CompilerValue> ASTBool::CodeGen()
{
	return m_Compiler->GenConstantValue(HazeValueType::Bool, m_Value);
}

ASTNumber::ASTNumber(Compiler* compiler, const SourceLocation& location, HazeValueType type, const HazeValue& value) 
	: ASTBase(compiler, location)
{
	m_DefineVariable.Type.PrimaryType = type;
	m_Value = value;
}

Share<CompilerValue> ASTNumber::CodeGen()
{
	return m_Compiler->GenConstantValue(m_DefineVariable.Type.PrimaryType, m_Value);
}

ASTStringText::ASTStringText(Compiler* compiler, const SourceLocation& location, HString& text) 
	: ASTBase(compiler, location), m_Text(Move(text)) {}

Share<CompilerValue> ASTStringText::CodeGen()
{
	return m_Compiler->GenStringVariable(m_Text);
}

ASTIdentifier::ASTIdentifier(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, HString& name,
	Unique<ASTBase> arrayIndexExpression, Unique<ASTBase> preAst, HString nameSpace)
	: ASTBase(compiler, location), m_SectionSignal(section), m_ArrayIndexExpression(Move(arrayIndexExpression)),
	m_PreAst(Move(preAst)), m_NameSpace(Move(nameSpace))
{
	m_DefineVariable.Name = Move(name);
}

Share<CompilerValue> ASTIdentifier::CodeGen()
{
	Share<CompilerValue> preValue = m_PreAst ? m_PreAst->CodeGen() : nullptr;
	Share<CompilerValue> retValue = GetNameValue();

	if (preValue)
	{
		if (preValue->IsAdvance() || preValue->IsElement())
		{
			if (retValue)
			{
				retValue = m_Compiler->CreateElementValue(preValue->IsElement() ? 
					DynamicCast<CompilerElementValue>(preValue)->CreateGetFunctionCall() : preValue, retValue);
			}
			else if (!m_DefineVariable.Name.empty())
			{
				retValue = m_Compiler->CreateElementValue(preValue->IsElement() ?
					DynamicCast<CompilerElementValue>(preValue)->CreateGetFunctionCall() : preValue, m_DefineVariable.Name);
			}
			else if (m_ArrayIndexExpression && preValue->IsElement())
			{
				retValue = m_Compiler->CreateElementValue(DynamicCast<CompilerElementValue>(preValue)->CreateGetFunctionCall(),
					m_ArrayIndexExpression->CodeGen());
				m_ArrayIndexExpression.release();
			}
			else
			{
				AST_ERR_W("变量<%s>有前置语法, 但是名字为空");
			}
		}
		else
		{
			AST_ERR_W("变量<%s>有前置语法, 但是不是复杂类型", m_DefineVariable.Name.c_str());
		}
	}

	if (m_ArrayIndexExpression)
	{
		retValue = m_Compiler->CreateElementValue(retValue->IsElement() ?
			DynamicCast<CompilerElementValue>(retValue)->CreateGetFunctionCall() : retValue,
			m_ArrayIndexExpression->CodeGen());
	}

	return retValue;
}

Share<CompilerValue> ASTIdentifier::GetNameValue()
{
	Share<CompilerValue> retValue = nullptr;

	if (!m_DefineVariable.Name.empty())
	{
		if (m_SectionSignal == HazeSectionSignal::Global)
		{
			retValue = m_Compiler->GetGlobalVariable(m_DefineVariable.Name);
		}
		else if (m_SectionSignal == HazeSectionSignal::Local)
		{
			retValue = m_Compiler->GetLocalVariable(m_DefineVariable.Name, m_NameSpace.empty() ? nullptr : &m_NameSpace);
			if (!retValue)
			{
				retValue = m_Compiler->GetGlobalVariable(m_DefineVariable.Name);
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

		if (!m_NameSpace.empty() && !retValue)
		{
			retValue = m_Compiler->GetEnumVariable(m_NameSpace, m_DefineVariable.Name);
		}
	}

	return retValue;
}

ASTFunctionCall::ASTFunctionCall(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	HString& name, V_Array<Unique<ASTBase>>& functionParam, Unique<ASTBase> classObj, HString nameSpaces)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Name(Move(name)),
	m_FunctionParam(Move(functionParam)), m_ClassObj(Move(classObj)), m_NameSpace(Move(nameSpaces)) {}

Share<CompilerValue> ASTFunctionCall::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	Share<CompilerFunction> function = nullptr;
	Share<CompilerValue> classObj = nullptr;

	if (m_ClassObj)
	{
		classObj = m_ClassObj->CodeGen();
		auto classValue = DynamicCast<CompilerClassValue>(classObj);
		if (classValue)
		{
			function = classValue->GetOwnerClass()->FindFunction(m_Name, m_NameSpace.empty() ? nullptr : &m_NameSpace);
		}
	}
	else
	{
		function = m_Compiler->GetFunction(m_Name);
	}

	V_Array<Share<CompilerValue>> param;

	for (int i = (int)m_FunctionParam.size() - 1; i >= 0; i--)
	{
		param.push_back(m_FunctionParam[i]->CodeGen());
		if (!param.back())
		{
			HAZE_LOG_ERR_W("函数<%s>中<%d>行调用<%s>第<%d>个参数错误!\n", m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str(), m_Location.Line, m_Name.c_str(), i + 1);
			return nullptr;
		}
	}
	
	Share<CompilerValue> ret = nullptr;
	if (function)
	{
		if (function->GetClass())
		{
			if (!classObj)
			{
				classObj = m_Compiler->GetCurrModule()->GetCurrFunction()->GetThisLocalVariable();
			}
		}

		ret = m_Compiler->CreateFunctionCall(function, param, classObj);
	}
	else
	{
		auto functionPointer = m_Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(m_Name, 
			m_NameSpace.empty() ? nullptr : &m_NameSpace);
		if (functionPointer && functionPointer->IsFunction())
		{
			ret = m_Compiler->CreateFunctionCall(functionPointer, param);
		}
		else if (classObj && classObj->IsElement())
		{
			auto elementValue = DynamicCast<CompilerElementValue>(classObj);
			ret = m_Compiler->CreateAdvanceTypeFunctionCall(elementValue->GetParentBaseType(), m_Name,
				param, elementValue);
		}
		else
		{
			AST_ERR_W("生成函数调用错误, 未能找到函数");
		}
	}

	if (!ret)
	{
		HAZE_LOG_ERR_W("生成函数调用<%s>错误, 检查函数名或 ( 符号是否与函数名在同一行\n", m_Name.c_str());
	}

	return ret;
}

//ASTClassAttr::ASTClassAttr(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
//	HString& classObjName, Unique<ASTBase>& preAst, HString& attrName, bool isFunction,
//	V_Array<Unique<ASTBase>>* functionParam)
//	: ASTBase(compiler, location), m_SectionSignal(section), m_PreAst(Move(preAst)), m_IsFunction(isFunction),
//	 m_AttrName(Move(attrName))
//{
//	m_DefineVariable.Name = Move(classObjName);
//	if (functionParam)
//	{
//		m_Params = Move(*functionParam);
//	}
//}
//
//Share<CompilerValue> ASTClassAttr::CodeGen()
//{
//	Share<CompilerValue> v;
//	if (!m_DefineVariable.Name.empty())
//	{
//		if (m_SectionSignal == HazeSectionSignal::Global)
//		{
//			v = m_Compiler->GetGlobalVariable(m_DefineVariable.Name);
//		}
//		else if (m_SectionSignal == HazeSectionSignal::Local)
//		{
//			v = m_Compiler->GetLocalVariable(m_DefineVariable.Name);
//			if (!v)
//			{
//				v = m_Compiler->GetGlobalVariable(m_DefineVariable.Name);
//			}
//			else if (v->IsClassMember())
//			{
//				auto currFunc = m_Compiler->GetCurrModule()->GetCurrFunction();
//				if (currFunc)
//				{
//					auto thisValue = currFunc->GetThisLocalVariable();
//					if (thisValue && thisValue->GetMember(m_DefineVariable.Name) == v)
//					{
//						v = m_Compiler->CreateGetClassMember(thisValue, m_DefineVariable.Name);
//					}
//				}
//			}
//		}
//	}
//	else
//	{
//		v = m_PreAst->CodeGen();
//	}
//
//	auto classValue = DynamicCast<CompilerClassValue>(v);
//	Share<CompilerValue> ret = nullptr;
//	if (m_IsFunction)
//	{
//		V_Array<Share<CompilerValue>> param;
//		for (int i = (int)m_Params.size() - 1; i >= 0; i--)
//		{
//			param.push_back(m_Params[i]->CodeGen());
//		}	
//
//		if (!classValue && IsAdvanceType(v->GetValueType().PrimaryType))
//		{
//			ret = m_Compiler->CreateAdvanceTypeFunctionCall(v->GetValueType().PrimaryType, m_AttrName, param,v);
//		}
//		else
//		{
//			ret = m_Compiler->CreateFunctionCall(classValue->GetOwnerClass()->FindFunction(m_AttrName), param, classValue);
//		}
//	}
//	else
//	{
//		ret = m_Compiler->CreateGetClassMember(classValue, m_AttrName);
//	}
//
//	return TryAssign(assignToAst, ret);
//}

ASTVariableDefine::ASTVariableDefine(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	const HazeDefineVariable& defineVar, Unique<ASTBase> expression)
	: ASTBase(compiler, location, defineVar), m_SectionSignal(section), m_Expression(Move(expression)) {}

Share<CompilerValue> ASTVariableDefine::CodeGen()
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);

	return m_Expression ? m_Compiler->CreateMov(var, GenExpressionValue(var)) : var;
}

Share<CompilerValue> ASTVariableDefine::GenExpressionValue(Share<CompilerValue> value)
{
	Share<CompilerValue> exprValue = nullptr;
	if (auto nullExpression = dynamic_cast<ASTNullPtr*>(m_Expression.get()))
	{
		nullExpression->SetDefineType(m_DefineVariable.Type);
	}
	exprValue = m_Expression->CodeGen();
	if (m_SectionSignal == HazeSectionSignal::Global && exprValue->GetModule() && value->GetModule())
	{
		if (exprValue->GetModule() != value->GetModule())
		{
			if (exprValue->GetModule()->IsImportModule(value->GetModule()) && value->GetModule()->IsImportModule(exprValue->GetModule()))
			{
				AST_ERR_W("不允许互相引用的模块的全局变量互相赋值");
				return nullptr;
			}
		}
	}

	return exprValue;
}

ASTVariableDefine_MultiVariable::ASTVariableDefine_MultiVariable(Compiler* compiler, const SourceLocation& location)
	: ASTBase(compiler, location)
{
	m_DefineVariable.Type = HazeValueType::MultiVariable;
	m_DefineVariable.Name = HAZE_MULTI_PARAM_NAME;
}

Share<CompilerValue> ASTVariableDefine_MultiVariable::CodeGen()
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	return m_Compiler->CreateVariableBySection(HazeSectionSignal::Local, currModule, currModule->GetCurrFunction(), m_DefineVariable, m_Location.Line);
}

ASTVariableDefine_Class::ASTVariableDefine_Class(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	const HazeDefineVariable& defineVar, Unique<ASTBase> expression, TemplateDefineTypes& templateTypes, V_Array<Unique<ASTBase>> params)
	: ASTVariableDefine(compiler, location, section, defineVar, Move(expression)), m_Params(Move(params))
{
	if (templateTypes.Types.size() > 0)
	{
		m_TemplateTypes.Types = Move(templateTypes.Types);
	}
}

Share<CompilerValue> ASTVariableDefine_Class::CodeGen()
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);

	return m_Expression ? m_Compiler->CreateMov(var, GenExpressionValue(var)) : var;
}

ASTVariableDefine_Array::ASTVariableDefine_Array(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	const HazeDefineVariable& defineVar, Unique<ASTBase> expression, TemplateDefineTypes& templateTypes, uint64 dimension)
	: ASTVariableDefine(compiler, location, section, defineVar, Move(expression)), m_ArrayDimension(dimension)
{
	if (templateTypes.Types.size() > 0)
	{
		m_TemplateTypes.Types = Move(templateTypes.Types);
	}
}

Share<CompilerValue> ASTVariableDefine_Array::CodeGen()
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrFunction(),
		m_DefineVariable, m_Location.Line, nullptr, m_ArrayDimension);

	return m_Expression ? m_Compiler->CreateMov(var, GenExpressionValue(var)) : var;
}

ASTVariableDefine_Function::ASTVariableDefine_Function(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	const HazeDefineVariable& defineVar, Unique<ASTBase> expression, TemplateDefineTypes& templateTypes)
	: ASTVariableDefine(compiler, location, section, defineVar, Move(expression))
{
	if (templateTypes.Types.size() > 0)
	{
		m_TemplateTypes.Types = Move(templateTypes.Types);
	}
}

Share<CompilerValue> ASTVariableDefine_Function::CodeGen()
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();

	V_Array<HazeDefineType> paramTypes;
	m_Compiler->GetRealTemplateTypes(m_TemplateTypes, paramTypes);
	auto retValue = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrFunction(),
		m_DefineVariable, m_Location.Line, nullptr, {}, &paramTypes);

	auto exprValue = m_Expression->CodeGen();
	if (!exprValue)
	{
		auto function = m_Compiler->GetCurrModule()->GetFunction(m_Expression->m_DefineVariable.Name);
		if (!function)
		{
			HAZE_LOG_ERR_W("未能找到变量<%s>,当前函数<%s>!\n", m_DefineVariable.Name.c_str(),
				m_SectionSignal == HazeSectionSignal::Local ? m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str() : H_TEXT("None"));
			return nullptr;
		}

		m_Compiler->CreatePointerToFunction(function, retValue);
	}
	
	return retValue;
}

ASTReturn::ASTReturn(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	:ASTBase(compiler, location), m_Expression(Move(expression)) {}

Share<CompilerValue> ASTReturn::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	auto& funcType = m_Compiler->GetCurrModule()->GetCurrFunction()->GetFunctionType();
	auto retReg = Compiler::GetRegister(RET_REGISTER);
	const_cast<HazeDefineType&>(retReg->GetValueType()) = funcType;

	Share<CompilerValue> retValue = m_Expression ? m_Expression->CodeGen() : m_Compiler->GetConstantValueInt(0);
	auto retValueType = retValue->GetValueType();
	if (retValue->IsEnum())
	{
		retValueType = DynamicCast<CompilerEnumValue>(retValue)->GetBaseType();
	}

	if (m_Expression ? retValueType == funcType : IsVoidType(funcType.PrimaryType))
	{
		return m_Compiler->CreateRet(retValue);
	}
	else
	{
		AST_ERR_W("返回值类型错误");
	}

	return nullptr;
}

ASTNew::ASTNew(Compiler* compiler, const SourceLocation& location, const HazeDefineVariable& DefineVar, V_Array<Unique<ASTBase>> countArrayExpression
	, V_Array<Unique<ASTBase>> constructorParam)
	: ASTBase(compiler, location, DefineVar), m_CountArrayExpression(Move(countArrayExpression)),
	m_ConstructorParam(Move(constructorParam)) {}

Share<CompilerValue> ASTNew::CodeGen()
{
	auto func = m_Compiler->GetCurrModule()->GetCurrFunction();

	V_Array<Share<CompilerValue>> countValue(m_CountArrayExpression.size());

	for (uint64 i = 0; i < m_CountArrayExpression.size(); i++)
	{
		countValue[i] = m_CountArrayExpression[i]->CodeGen();
	}

	auto value = m_Compiler->CreateNew(func, m_DefineVariable.Type, &countValue);
	

	//new申请内存后，若是类的话，需要调用构造函数
	if (value->GetValueType().NeedCustomName())
	{
		auto newClass = m_Compiler->GetCurrModule()->GetClass(*value->GetValueType().CustomName);
		if (countValue.size() > 0)
		{
			//需要初始化多个类对象，类的参数个数必须为0
			/*auto function = newClass->FindFunction(*value->GetValueType().CustomName);
			auto arrayConstructorFunc = m_Compiler->GetFunction(HAZE_OBJECT_ARRAY_CONSTRUCTOR);
			V_Array<Share<CompilerValue>> param;

			value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(value->GetValueType()), value);

			if (function->HasExceptThisParam())
			{
				HAZE_LOG_ERR_W("生成类对象数组的构造函数必须是无参数的!\n");
				return value;
			}

			auto constantValue = m_Compiler->GetConstantValueUint64(1);
			auto objCount = m_Compiler->CreateMov(
				m_Compiler->GetTempRegister(constantValue->GetValueType()), constantValue);
			for (uint64 i = 0; i < countValue.size(); i++)
			{
				m_Compiler->CreateMul(objCount, objCount, countValue[i]);
			}

			V_Array<Share<CompilerValue>> params;
			params.push_back(objCount);
			params.push_back(m_Compiler->GetConstantValueUint64(newClass->GetDataSize()));
			params.push_back(m_Compiler->CreatePointerToFunction(function, m_Compiler->GetTempRegister(HazeValueType::UInt64)));
			params.push_back(m_Compiler->CreateMov(m_Compiler->GetTempRegister(HazeValueType::UInt64), value));

			m_Compiler->CreateFunctionCall(arrayConstructorFunc.first, params);*/
		}
		else
		{
			HAZE_LOG_INFO(H_TEXT("考虑new时也要调用所有父类的构造函数\n"));
			auto function = newClass->FindFunction(*value->GetValueType().CustomName, nullptr);
			V_Array<Share<CompilerValue>> param;

			//构造参数
			for (int i = (int)m_ConstructorParam.size() - 1; i >= 0; i--)
			{
				param.push_back(m_ConstructorParam[i]->CodeGen());
			}

			value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(value->GetValueType()), value);
			m_Compiler->CreateFunctionCall(function, param, value);
		}
	}


	return value;
}

ASTGetAddress::ASTGetAddress(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression)) {}

Share<CompilerValue> ASTGetAddress::CodeGen()
{
	Share<CompilerValue> retValue = nullptr;
	if (m_Expression)
	{
		retValue = m_Expression->CodeGen();
	}

	if (!retValue)
	{
		//获得函数地址
		auto function = m_Compiler->GetCurrModule()->GetFunction(m_Expression->GetName());
		if (function)
		{
			retValue = m_Compiler->CreatePointerToFunction(function, nullptr);
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

ASTNeg::ASTNeg(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isNumberNeg)
	: ASTBase(compiler, location), m_Expression(Move(expression)), m_IsNumberNeg(isNumberNeg) {}

Share<CompilerValue> ASTNeg::CodeGen()
{
	Share<CompilerValue> ret = nullptr;
	if (m_IsNumberNeg)
	{
		ret = m_Compiler->CreateNeg(nullptr, m_Expression->CodeGen());
	}
	else
	{
		ret = m_Compiler->CreateBitNeg(nullptr, m_Expression->CodeGen());
	}
	
	return ret;
}

ASTNullPtr::ASTNullPtr(Compiler* compiler, const SourceLocation& location) : ASTBase(compiler, location) {}

Share<CompilerValue> ASTNullPtr::CodeGen()
{
	return m_Compiler->GetNullPtr(m_DefineVariable.Type);
}

void ASTNullPtr::SetDefineType(const HazeDefineType& type)
{
	m_DefineVariable.Type = type;
}

ASTNot::ASTNot(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression)) {}

Share<CompilerValue> ASTNot::CodeGen()
{
	auto value = m_Expression->CodeGen();
	return m_Compiler->CreateNot(nullptr, value);
}


ASTInc::ASTInc(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreInc)
	: ASTBase(compiler, location), m_IsPreInc(isPreInc), m_Expression(Move(expression)) {}

Share<CompilerValue> ASTInc::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	return m_Compiler->CreateInc(m_Expression->CodeGen(), m_IsPreInc);
}

ASTDec::ASTDec(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreDec)
	: ASTBase(compiler, location), m_IsPreDec(isPreDec), m_Expression(Move(expression)) {}

Share<CompilerValue> ASTDec::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	return m_Compiler->CreateDec(m_Expression->CodeGen(), m_IsPreDec);
}


ASTDataSection::ASTDataSection(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, V_Array<Unique<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(Move(expressions)) {}

Share<CompilerValue> ASTDataSection::CodeGen()
{
	for (size_t i = 0; i < m_Expressions.size(); i++)
	{
		m_Expressions[i]->CodeGen();
	}

	return nullptr;
}

ASTMultiExpression::ASTMultiExpression(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	V_Array<Unique<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(Move(expressions)) {}

Share<CompilerValue> ASTMultiExpression::CodeGen()
{
	//要考虑清楚无效的临时寄存器中引用的情况，全局变量的初始不用考虑，会在执行进入主函数字节码之前初始化完成
	auto func = m_Compiler->GetCurrModule()->GetCurrFunction();
	if (!func)
	{
		AST_ERR_W("执行多行式AST错误, 未能找打当前运行函数");
	}

	for (size_t i = 0; i < m_Expressions.size(); i++)
	{
		m_Expressions[i]->CodeGen();
		func->TryClearTempRegister();
	}

	return nullptr;
}

ASTBinaryExpression::ASTBinaryExpression(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	HazeToken operatorToken, Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST)
	:ASTBase(compiler, location), m_SectionSignal(section), m_OperatorToken(operatorToken), m_LeftAST(Move(leftAST)),
	m_RightAST(Move(rightAST)), m_LeftBlock(nullptr), m_RightBlock(nullptr), m_DefaultBlock(nullptr), m_AssignToAst(nullptr) {}

void ASTBinaryExpression::SetLeftAndRightBlock(CompilerBlock* leftJmpBlock, CompilerBlock* rightJmpBlock)
{
	m_LeftBlock = leftJmpBlock;
	m_RightBlock = rightJmpBlock;
}

Share<CompilerValue> ASTBinaryExpression::CodeGen()
{
#define BINARYOPER(OPER) m_Compiler->Create##OPER(m_AssignToAst ? m_AssignToAst->CodeGen() : nullptr, left, right);

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
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return BINARYOPER(Add);
		}
		case HazeToken::Sub:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return BINARYOPER(Sub);
		}
		case HazeToken::Mul:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return BINARYOPER(Mul);
		}
		case HazeToken::Div:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return BINARYOPER(Div);
		}
		case HazeToken::Inc:
			return m_Compiler->CreateInc(m_LeftAST->CodeGen(), false);
		case HazeToken::Dec:
			return m_Compiler->CreateDec(m_LeftAST->CodeGen(), false);
		case HazeToken::Mod:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateMod(nullptr, left, right);
		}
		case HazeToken::Not:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateNot(left, right);
		}
		case HazeToken::Shl:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return BINARYOPER(Shl);
		}
		case HazeToken::Shr:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return BINARYOPER(Shr);
		}
		case HazeToken::Assign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateMov(left, right);
		}
		case HazeToken::BitAnd:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return BINARYOPER(BitAnd);
		}
		case HazeToken::BitOr:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return BINARYOPER(BitOr);
		}
		case HazeToken::BitXor:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return BINARYOPER(BitXor);
		}
		case HazeToken::AddAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateAdd(left, left, right);
		}
		case HazeToken::SubAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateSub(left, left, right);
		}
		case HazeToken::MulAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateMul(left, left, right);
		}
		case HazeToken::DivAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateDiv(left, left, right);
		}
		case HazeToken::ModAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateMod(left, left, right);
		}
		case HazeToken::BitAndAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateBitAnd(left, left, right);
		}
		case HazeToken::BitOrAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateBitOr(left, left, right);
		}
		case HazeToken::BitXorAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateBitXor(left, left, right);
		}
		case HazeToken::ShlAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateShl(left, left, right);
		}
		case HazeToken::ShrAssign:
		{
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			return m_Compiler->CreateShr(left, left, right);
		}
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
			auto left = m_LeftAST->CodeGen();
			auto right = m_RightAST->CodeGen();
			auto retValue = m_Compiler->CreateIntCmp(left, right);
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

ASTThreeExpression::ASTThreeExpression(Compiler* compiler, const SourceLocation& location
	, Unique<ASTBase>& conditionAST, Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST)
	: ASTBase(compiler, location), m_ConditionAST(Move(conditionAST)), m_LeftAST(Move(leftAST)), m_RightAST(Move(rightAST)) {}

Share<CompilerValue> ASTThreeExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();

	auto parentBlock = m_Compiler->GetInsertBlock();

	auto defauleBlock = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
	auto blockLeft = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
	auto blockRight = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);

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

	m_Compiler->SetInsertBlock(blockLeft);
	auto value = m_LeftAST->CodeGen();
	auto retValue = m_Compiler->GetTempRegister(value);
	m_Compiler->CreateMov(retValue, value);
	m_Compiler->CreateJmpToBlock(defauleBlock);

	m_Compiler->SetInsertBlock(blockRight);
	value = m_RightAST->CodeGen();
	m_Compiler->CreateMov(retValue, value);
	m_Compiler->CreateJmpToBlock(defauleBlock);

	m_Compiler->SetInsertBlock(defauleBlock);

	return retValue;
}

ASTImportModule::ASTImportModule(Compiler* compiler, const SourceLocation& location, const HString& modulepath)
	: ASTBase(compiler, location), m_ModulePath(modulepath) {}

Share<CompilerValue> ASTImportModule::CodeGen()
{
	m_Compiler->AddImportModuleToCurrModule(m_Compiler->ParseModule(m_ModulePath));
	return nullptr;
}

ASTBreakExpression::ASTBreakExpression(Compiler* compiler, const SourceLocation& location) 
	: ASTBase(compiler, location) {}

Share<CompilerValue> ASTBreakExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	m_Compiler->CreateJmpToBlock(m_Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopEnd()->GetShared());
	return nullptr;
}

ASTContinueExpression::ASTContinueExpression(Compiler* compiler, const SourceLocation& location)
	: ASTBase(compiler, location) {}

Share<CompilerValue> ASTContinueExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	m_Compiler->CreateJmpToBlock(m_Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopStep()->GetShared());
	return nullptr;
}

ASTIfExpression::ASTIfExpression(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& condition,
	Unique<ASTBase>& ifExpression, Unique<ASTBase>& elseExpression)
	: ASTBase(compiler, location),m_Condition(Move(condition)), m_IfExpression(Move(ifExpression)),
		m_ElseExpression(Move(elseExpression)) {}

Share<CompilerValue> ASTIfExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();

	auto parentBlock = m_Compiler->GetInsertBlock();
	auto ifThenBlock = CompilerBlock::CreateBaseBlock(function->GenIfThenBlockName(), function, parentBlock);
	auto elseBlock = m_ElseExpression ? CompilerBlock::CreateBaseBlock(function->GenElseBlockName(), function, parentBlock) : nullptr;
	auto nextBlock = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);

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

ASTWhileExpression::ASTWhileExpression(Compiler* compiler, const SourceLocation& location, 
	Unique<ASTBase>& condition, Unique<ASTBase>& multiExpression)
	:ASTBase(compiler, location), m_Condition(Move(condition)), m_MultiExpression(Move(multiExpression)) {}

Share<CompilerValue> ASTWhileExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();

	auto parentBlock = m_Compiler->GetInsertBlock();
	auto whileBlock = CompilerBlock::CreateBaseBlock(function->GenWhileBlockName(), function, parentBlock);
	auto nextBlock = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);

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

ASTForExpression::ASTForExpression(Compiler* compiler, const SourceLocation& location,
	Unique<ASTBase>& initExpression, Unique<ASTBase>& conditionExpression, 
	Unique<ASTBase>& stepExpression, Unique<ASTBase>& multiExpression)
	: ASTBase(compiler, location), m_InitExpression(Move(initExpression)), m_ConditionExpression(Move(conditionExpression)),
		m_StepExpression(Move(stepExpression)), m_MultiExpression(Move(multiExpression)) {}

Share<CompilerValue> ASTForExpression::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();
	auto parentBlock = m_Compiler->GetInsertBlock();

	auto forConditionBlock = CompilerBlock::CreateBaseBlock(function->GenForConditionBlockName(), function, parentBlock);
	auto loopBlock = CompilerBlock::CreateBaseBlock(function->GenLoopBlockName(), function, parentBlock);
	auto forStepBlock = CompilerBlock::CreateBaseBlock(function->GenForStepBlockName(), function, parentBlock);
	auto endLoopBlock = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
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

ASTCast::ASTCast(Compiler* compiler, const SourceLocation& location, HazeDefineType& type, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression))
{
	m_DefineVariable.Type = type;
}

Share<CompilerValue> ASTCast::CodeGen()
{
	return m_Compiler->CreateCast(m_DefineVariable.Type, m_Expression->CodeGen());
}

ASTSizeOf::ASTSizeOf(Compiler* compiler, const SourceLocation& location, const HazeDefineType& type, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression))
{
	m_DefineVariable.Type = type;
}

Share<CompilerValue> ASTSizeOf::CodeGen()
{
	Share<CompilerValue> ret = nullptr;
	if (m_Expression)
	{
		ret = m_Compiler->GetConstantValueUint64(m_Expression->CodeGen()->GetSize());
	}
	else
	{
		ret = m_Compiler->GetConstantValueUint64(m_DefineVariable.Type.GetCompilerTypeSize());
	}

	return ret;
}
