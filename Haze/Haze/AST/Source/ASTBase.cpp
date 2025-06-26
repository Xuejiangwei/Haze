#include "HazePch.h"
#include "ASTBase.h"
#include "HazeLogDefine.h"

#include "CompilerHelper.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerClass.h"
#include "CompilerBlock.h"
#include "CompilerFunction.h"
#include "CompilerClosureFunction.h"
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
				AST_ERR_W("����<%s>��ǰ���﷨, ��������Ϊ��");
			}
		}
		else
		{
			AST_ERR_W("����<%s>��ǰ���﷨, ���ǲ��Ǹ�������", m_DefineVariable.Name.c_str());
		}
	}

	if (m_ArrayIndexExpression)
	{
		if (retValue)
		{
			retValue = m_Compiler->CreateElementValue(retValue->IsElement() ?
				DynamicCast<CompilerElementValue>(retValue)->CreateGetFunctionCall() : retValue,
				m_ArrayIndexExpression->CodeGen());
		}
		else
		{
			AST_ERR_W("����<%s>δ���ҵ�", m_DefineVariable.Name.c_str());
		}
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
		else if (m_SectionSignal == HazeSectionSignal::Closure)
		{
			retValue = m_Compiler->GetClosureVariable(m_DefineVariable.Name);
			if (!retValue)
			{
				retValue = m_Compiler->GetLocalVariable(m_DefineVariable.Name, m_NameSpace.empty() ? nullptr : &m_NameSpace);
				if (!retValue)
				{
					retValue = m_Compiler->GetGlobalVariable(m_DefineVariable.Name);
				}
				else
				{
					m_Compiler->GetCurrModule()->ClosureAddLocalRef(retValue, m_DefineVariable.Name);
				}
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
	auto& currModule = m_Compiler->GetCurrModule();

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
			AST_ERR_W("����<%s>��<%d>�е���<%s>��<%d>����������", currModule->GetCurrFunction()->GetName().c_str(), m_Location.Line, m_Name.c_str(), i + 1);
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
				classObj = currModule->GetCurrFunction()->GetThisLocalVariable();
				if (!m_NameSpace.empty())
				{
					function = DynamicCast<CompilerClassValue>(classObj)->GetOwnerClass()->FindFunction(m_Name, &m_NameSpace);
				}
			}
		}

		ret = m_Compiler->CreateFunctionCall(function, param, classObj, m_NameSpace.empty() ? nullptr : &m_NameSpace);
	}
	else
	{
		Share<CompilerValue> functionValue = nullptr;
		functionValue = currModule->GetClosureVariable(m_Name, true);
		if (!functionValue)
		{
			functionValue = currModule->GetCurrFunction()->GetLocalVariable(m_Name, m_NameSpace.empty() ? nullptr : &m_NameSpace);
		}

		if (functionValue)
		{
			if (functionValue->IsFunction())
			{
				ret = m_Compiler->CreateFunctionCall(functionValue, param);
			}
			else if (functionValue->IsClosure())
			{
				ret = m_Compiler->CreateAdvanceTypeFunctionCall(HazeValueType::Closure, HAZE_CUSTOM_CALL_FUNCTION, param, functionValue);
			}
		}
		else if (classObj)
		{
			if (classObj->IsElement())
			{
				auto elementValue = DynamicCast<CompilerElementValue>(classObj);
				ret = m_Compiler->CreateAdvanceTypeFunctionCall(elementValue->GetParentBaseType(), m_Name, param, elementValue);
			}
			else if (classObj->IsDynamicClass())
			{
				ret = m_Compiler->CreateAdvanceTypeFunctionCall(HazeValueType::DynamicClass, m_Name, param, classObj);
			}
			else if (classObj->IsObjectBase())
			{
				ret = m_Compiler->CreateAdvanceTypeFunctionCall(HazeValueType::ObjectBase, m_Name, param, classObj);
			}
			else
			{
				AST_ERR_W("���ɺ������ô���, <%s>���ʹ���", m_Name.c_str());
			}
		}
		else
		{
			AST_ERR_W("���ɺ������ô���, δ���ҵ�����");
		}
	}

	if (!ret)
	{
		AST_ERR_W("���ɺ�������<%s>����, ��麯������ ( �����Ƿ��뺯������ͬһ��", m_Name.c_str());
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
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);
	return TryAssign(var, H_TEXT("����"));
}

Share<CompilerValue> ASTVariableDefine::GenExpressionValue(Share<CompilerValue> value)
{
	Share<CompilerValue> exprValue = nullptr;
	/*if (auto nullExpression = dynamic_cast<ASTNullPtr*>(m_Expression.get()))
	{
		nullExpression->SetDefineType(m_DefineVariable.Type);
	}*/

	exprValue = m_Expression->CodeGen();
	if (m_SectionSignal == HazeSectionSignal::Global && exprValue->GetModule() && value->GetModule())
	{
		if (exprValue->GetModule() != value->GetModule())
		{
			if (exprValue->GetModule()->IsImportModule(value->GetModule()) && value->GetModule()->IsImportModule(exprValue->GetModule()))
			{
				AST_ERR_W("�����������õ�ģ���ȫ�ֱ������ำֵ");
				return nullptr;
			}
		}
	}

	return exprValue;
}

Share<CompilerValue> ASTVariableDefine::TryAssign(Share<CompilerValue> var, const x_HChar* errorPrefix)
{
	HAZE_TO_DO(��Ҫ���Ƕ��Ƕ�ױհ�������������հ������еı�����ȫ�ֱ������� ��Ҫ�����ຯ��������������);
	if (var)
	{
		if (m_Expression)
		{
			auto v = GenExpressionValue(var);
			if (v)
			{
				return m_Compiler->CreateMov(var, v);
			}
			else
			{
				AST_ERR_W("%s<%s>���帳ֵ�ұ߱��ʽ����", errorPrefix, m_DefineVariable.Name.c_str());
			}
		}
		else
		{
			return var;
		}

	}
	else
	{
		AST_ERR_W("%s<%s>�������", errorPrefix, m_DefineVariable.Name.c_str());
	}

	return nullptr;
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
	return m_Compiler->CreateVariableBySection(HazeSectionSignal::Local, currModule, currModule->GetCurrClosureOrFunction(), m_DefineVariable, m_Location.Line);
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
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);
	return TryAssign(var, H_TEXT("�����"));
}

ASTVariableDefine_Array::ASTVariableDefine_Array(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	const HazeDefineVariable& defineVar, Unique<ASTBase> expression, TemplateDefineTypes& templateTypes, x_uint64 dimension)
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
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr, m_ArrayDimension);

	return TryAssign(var, H_TEXT("�������"));
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
	V_Array<HazeDefineType> paramTypes(m_TemplateTypes.Types.size() + 1);

	//�����������õ���0��
	paramTypes[0] = m_DefineVariable.Type;
	//m_Compiler->GetRealTemplateTypes(m_TemplateTypes, paramTypes);

	/*for (x_uint64 i = 0; i < m_TemplateTypes.Types.size(); i++)
	{
		paramTypes[i + 1] = *m_TemplateTypes.Types[i].Type;
	}*/

	m_DefineVariable.Type = HazeValueType::Function;
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr, {}/*, &m_TemplateTypes*/);
	
	return TryAssign(var, H_TEXT("����ָ�����"));
}

ASTVariableDefine_ObjectBase::ASTVariableDefine_ObjectBase(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	const HazeDefineVariable& defineVar, Unique<ASTBase>& expression)
	: ASTVariableDefine(compiler, location, section, defineVar, Move(expression))
{
}

Share<CompilerValue> ASTVariableDefine_ObjectBase::CodeGen()
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr, {}, nullptr);

	return TryAssign(var, H_TEXT("�����������"));
}

ASTVariableDefine_Hash::ASTVariableDefine_Hash(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	const HazeDefineVariable& defineVar, Unique<ASTBase>& expression, TemplateDefineTypes& templateTypes)
	: ASTVariableDefine(compiler, location, section, defineVar, Move(expression)), m_TemplateTypes(Move(templateTypes))
{
}

Share<CompilerValue> ASTVariableDefine_Hash::CodeGen()
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	V_Array<HazeDefineType> paramTypes(m_TemplateTypes.Types.size());
	/*for (x_uint64 i = 0; i < m_TemplateTypes.Types.size(); i++)
	{
		paramTypes[i] = *m_TemplateTypes.Types[i].Type;
	}*/

	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr, {}, &m_TemplateTypes);

	return TryAssign(var, H_TEXT("����ָ�����"));
}

ASTVariableDefine_Closure::ASTVariableDefine_Closure(Compiler* compiler, const SourceLocation& location, const SourceLocation& startLocation, const SourceLocation& endLocation, HazeSectionSignal section, const HazeDefineVariable& defineVar, Unique<ASTBase>& expression, TemplateDefineTypes& templateTypes, V_Array<Unique<ASTBase>>& params)
	: ASTVariableDefine_Function(compiler, location, section, defineVar, Move(expression), templateTypes), m_Params(Move(params)), m_StartLocation(startLocation), m_EndLocation(endLocation)
{
}

Share<CompilerValue> ASTVariableDefine_Closure::CodeGen()
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();

	Share<CompilerClosureFunction> closureFunction = nullptr;
	Share<CompilerClass> currClass = nullptr;

	V_Array<HazeDefineVariable> paramDefines(m_Params.size());
	for (size_t i = 0; i < m_Params.size(); i++)
	{
		paramDefines[i] = m_Params[i]->GetDefine();
	}

	closureFunction = currModule->CreateClosureFunction(m_DefineVariable.Type, paramDefines);
	closureFunction->SetStartEndLine(m_StartLocation.Line, m_EndLocation.Line);
	m_Compiler->SetInsertBlock(closureFunction->GetEntryBlock());

	for (int i = (int)m_Params.size() - 1; i >= 0; i--)
	{
		currModule->BeginCreateFunctionParamVariable();
		m_Params[i]->CodeGen();
		currModule->EndCreateFunctionParamVariable();
	}

	if (m_Expression)
	{
		m_Expression->CodeGen();
	}

	if (closureFunction == currModule->GetCurrClosure())
	{
		currModule->FinishClosure();
	}
	else
	{
		//auto& m_Location = m_EndLocation;
		AST_ERR_W("������������<%s>��������, ���ǵ�ǰģ������ĺ���<%s>", m_DefineVariable.Name.c_str(), currModule->GetCurrClosure()->GetName().c_str());
	}

	auto closureValue = m_Compiler->CreateNew(HazeValueType::Closure, nullptr, nullptr, closureFunction);
	V_Array<HazeDefineType> paramTypes(m_TemplateTypes.Types.size() + 1);

	//�����������õ���0��
	m_TemplateTypes.Types.push_back({ false, nullptr, MakeShare<HazeNewDefineType>(m_DefineVariable.Type, 0) });
	//m_Compiler->GetRealTemplateTypes(m_TemplateTypes, paramTypes);

	/*for (x_uint64 i = 0; i < m_TemplateTypes.Types.size(); i++)
	{
		paramTypes[i + 1] = *m_TemplateTypes.Types[i].Type;
	}*/

	m_DefineVariable.Type = HazeValueType::Closure;
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr, {}, &m_TemplateTypes);

	return m_Compiler->CreateMov(var, closureValue);
}

ASTReturn::ASTReturn(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	:ASTBase(compiler, location), m_Expression(Move(expression)) {}

Share<CompilerValue> ASTReturn::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	
	auto& funcType = m_Compiler->GetCurrModule()->GetCurrClosureOrFunction()->GetFunctionType();
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
		AST_ERR_W("����ֵ���ʹ���");
	}

	return nullptr;
}

ASTNew::ASTNew(Compiler* compiler, const SourceLocation& location, const HazeDefineVariable& DefineVar, TemplateDefineTypes& templateTypes, V_Array<Unique<ASTBase>> countArrayExpression
	, V_Array<Unique<ASTBase>> constructorParam)
	: ASTBase(compiler, location, DefineVar), m_CountArrayExpression(Move(countArrayExpression)),
	m_ConstructorParam(Move(constructorParam))
{
	if (templateTypes.Types.size() > 0)
	{
		m_TemplateTypes.Types = Move(templateTypes.Types);
	}
}

Share<CompilerValue> ASTNew::CodeGen()
{
	if (!IsAdvanceType(m_DefineVariable.Type.PrimaryType) || IsDynamicClassType(m_DefineVariable.Type.PrimaryType))
	{
		AST_ERR_W("�������ʹ���, ֻ������<��><����><�ַ���>");
		return nullptr;
	}

	//auto func = m_Compiler->GetCurrModule()->GetCurrFunction();

	V_Array<Share<CompilerValue>> countValue(m_CountArrayExpression.size());

	for (x_uint64 i = 0; i < m_CountArrayExpression.size(); i++)
	{
		countValue[i] = m_CountArrayExpression[i]->CodeGen();
	}

	Share<CompilerValue> value = m_Compiler->CreateNew(m_DefineVariable.Type, &countValue, &m_TemplateTypes);
	

	//new�����ڴ��������Ļ�����Ҫ���ù��캯��
	if (value->GetValueType().NeedCustomName())
	{
		auto newClass = m_Compiler->GetCurrModule()->GetClass(*value->GetValueType().CustomName);
		if (countValue.size() > 0)
		{
			//��Ҫ��ʼ������������Ĳ�����������Ϊ0
			/*auto function = newClass->FindFunction(*value->GetValueType().CustomName);
			auto arrayConstructorFunc = m_Compiler->GetFunction(HAZE_OBJECT_ARRAY_CONSTRUCTOR);
			V_Array<Share<CompilerValue>> param;

			value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(value->GetValueType()), value);

			if (function->HasExceptThisParam())
			{
				HAZE_LOG_ERR_W("�������������Ĺ��캯���������޲�����!\n");
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
			HAZE_LOG_INFO(H_TEXT("����newʱҲҪ�������и���Ĺ��캯��\n"));
			auto function = newClass->FindFunction(*value->GetValueType().CustomName, nullptr);
			value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(value->GetValueType()), value);

			if (function)
			{
				V_Array<Share<CompilerValue>> param;

				//�������
				for (int i = (int)m_ConstructorParam.size() - 1; i >= 0; i--)
				{
					param.push_back(m_ConstructorParam[i]->CodeGen());
				}
				m_Compiler->CreateFunctionCall(function, param, value);
			}
		}
	}
	else if (value->IsObjectBase())
	{
		if (m_ConstructorParam.size() == 1 && m_TemplateTypes.Types.size() == 1)
		{
			const_cast<HazeDefineType&>(value->GetValueType()).SecondaryType = m_TemplateTypes.Types[0].Type->BaseType.PrimaryType;
			m_Compiler->CreateAdvanceTypeFunctionCall(HazeValueType::ObjectBase, HAZE_OBJECT_BASE_CONSTRUCTOR, { m_ConstructorParam[0]->CodeGen() }, value);
		}
		else
		{
			AST_ERR_W("�������Ͷ����������ֻ��Ϊ1��");
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
		//��ú�����ַ
		auto function = m_Compiler->GetCurrModule()->GetFunction(m_Expression->GetName());
		if (function)
		{
			retValue = m_Compiler->CreatePointerToFunction(function, m_Compiler->GetTempRegister(HazeValueType::Function));
		}
		else if (!m_Expression->GetDefine().Name.empty() && dynamic_cast<ASTIdentifier*>(m_Expression.get()))
		{
			// ��ĳ�Ա����
			auto& nameSpace = dynamic_cast<ASTIdentifier*>(m_Expression.get())->GetNameSpace();
			auto compilerClass = m_Compiler->GetCurrModule()->GetClass(nameSpace);
			if (compilerClass)
			{
				function = compilerClass->FindFunction(m_Expression->GetName(), nullptr);
				if (function)
				{
					retValue = m_Compiler->CreatePointerToFunction(function, m_Compiler->GetTempRegister(HazeValueType::Function));
				}
				else
				{
					AST_ERR_W("��ú�����ַ<%s>����, δ���ҵ�������<%s>�ĳ�Ա����<%s>", m_Expression->GetName(), nameSpace.c_str(), m_Expression->GetName());
				}
			}
			else
			{
				AST_ERR_W("��ú�����ַ<%s>����, δ���ҵ�������<%s>", m_Expression->GetDefine().Name.c_str(), m_Expression->GetName());
			}
		}
		else
		{
			AST_ERR_W("δ���ҵ�����<%s>ȥ��ú�����ַ", m_Expression->GetName());
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
	auto v = m_Expression->CodeGen();
	if (v)
	{
		if (m_IsNumberNeg)
		{
			return m_Compiler->CreateNeg(nullptr, v);
		}
		else
		{
			return m_Compiler->CreateBitNeg(nullptr, v);
		}
	}
	else
	{
		AST_ERR_W("<%s>ȡ������", m_Expression->GetName());
	}
	
	return nullptr;
}

ASTNullPtr::ASTNullPtr(Compiler* compiler, const SourceLocation& location) : ASTBase(compiler, location) {}

Share<CompilerValue> ASTNullPtr::CodeGen()
{
	return m_Compiler->GetNullPtr(/*m_DefineVariable.Type*/HazeValueType::UInt64);
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
	if (value)
	{
		return m_Compiler->CreateNot(nullptr, value);
	}
	else
	{
		AST_ERR_W("<%s>ȡ������", m_Expression->GetName());
	}

	return nullptr;
}


ASTInc::ASTInc(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreInc)
	: ASTBase(compiler, location), m_IsPreInc(isPreInc), m_Expression(Move(expression)) {}

Share<CompilerValue> ASTInc::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	auto value = m_Expression->CodeGen();
	if (value)
	{
		return m_Compiler->CreateInc(value, m_IsPreInc);
	}
	else
	{
		AST_ERR_W("<%s>�ԼӴ���", m_Expression->GetName());
	}

	return nullptr;
}

ASTDec::ASTDec(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreDec)
	: ASTBase(compiler, location), m_IsPreDec(isPreDec), m_Expression(Move(expression)) {}

Share<CompilerValue> ASTDec::CodeGen()
{
	m_Compiler->InsertLineCount(m_Location.Line);
	auto value = m_Expression->CodeGen();
	if (value)
	{
		return m_Compiler->CreateDec(value, m_IsPreDec);

	}
	else
	{
		AST_ERR_W("<%s>�Լ�����", m_Expression->GetName());
	}

	return nullptr;
}


ASTDataSection::ASTDataSection(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, V_Array<Unique<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(Move(expressions)) {}

Share<CompilerValue> ASTDataSection::CodeGen()
{
	m_Compiler->GetCurrModule()->BeginGlobalDataDefine();

	for (size_t i = 0; i < m_Expressions.size(); i++)
	{
		m_Expressions[i]->CodeGen();
	}

	m_Compiler->GetCurrModule()->EndGlobalDataDefine();
	return nullptr;
}

ASTMultiExpression::ASTMultiExpression(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	V_Array<Unique<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(Move(expressions)) {}

Share<CompilerValue> ASTMultiExpression::CodeGen()
{
	//Ҫ���������Ч����ʱ�Ĵ��������õ������ȫ�ֱ����ĳ�ʼ���ÿ��ǣ�����ִ�н����������ֽ���֮ǰ��ʼ�����
	auto func = m_Compiler->GetCurrModule()->GetCurrFunction();
	if (!func)
	{
		AST_ERR_W("ִ�ж���ʽAST����, δ���Ҵ�ǰ���к���");
	}

	for (size_t i = 0; i < m_Expressions.size(); i++)
	{
		m_Expressions[i]->CodeGen();
		func->TryClearTempRegister();

		if (m_Compiler->IsCompileError())
		{
			break;
		}
	}

	return nullptr;
}

ASTBinaryExpression::ASTBinaryExpression(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	HazeToken operatorToken, Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST)
	: ASTBase(compiler, location), m_SectionSignal(section), m_OperatorToken(operatorToken), m_LeftAST(Move(leftAST)),
	m_RightAST(Move(rightAST)), m_LeftBlock(nullptr), m_RightBlock(nullptr), m_ParentAst(nullptr), m_ShortCircuitBlock(nullptr), m_AssignToAst(nullptr) {}

void ASTBinaryExpression::SetLeftAndRightBlock(CompilerBlock* leftJmpBlock, CompilerBlock* rightJmpBlock, ASTBinaryExpression* parentAst)
{
	m_LeftBlock = leftJmpBlock;
	m_RightBlock = rightJmpBlock;
	m_ParentAst = parentAst;
}

CompilerBlock* ASTBinaryExpression::TryShortCircuit(ASTBinaryExpression* ast)
{
	if (m_ParentAst)
	{
		if (m_ParentAst->m_OperatorToken == HazeToken::Or && m_ParentAst->m_RightAST && m_ParentAst->m_RightAST.get() != ast)
		{
			if (!m_ParentAst->m_ShortCircuitBlock)
			{
				auto function = m_Compiler->GetCurrModule()->GetCurrFunction();
				auto parentBlock = m_Compiler->GetInsertBlock();
				m_ParentAst->m_ShortCircuitBlock = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
			}

			return m_ParentAst->m_ShortCircuitBlock.get();
		}
		else if (m_ParentAst->m_ParentAst)
		{
			return m_ParentAst->TryShortCircuit(ast);
		}
	}

	return nullptr;
}

void ASTBinaryExpression::SetChildBlock()
{
	auto rightExp = dynamic_cast<ASTBinaryExpression*>(m_RightAST.get());
	if (rightExp && IsAndOrToken(rightExp->m_OperatorToken))
	{
		rightExp->SetLeftAndRightBlock(m_LeftBlock, m_RightBlock, this);
	}

	auto leftExp = dynamic_cast<ASTBinaryExpression*>(m_LeftAST.get());
	if (leftExp && IsAndOrToken(leftExp->m_OperatorToken))
	{
		if (IsAndToken(leftExp->m_OperatorToken))
		{
			leftExp->SetLeftAndRightBlock(nullptr, m_RightBlock, this);

		}
		else if (IsOrToken(leftExp->m_OperatorToken))
		{
			leftExp->SetLeftAndRightBlock(m_LeftBlock, nullptr, this);
		}
	}
}

Share<CompilerValue> ASTBinaryExpression::CodeGen()
{
#define BINARYOPER(OPER) m_Compiler->Create##OPER(m_AssignToAst ? m_AssignToAst->CodeGen() : nullptr, left, right);
#define ASSERT_GEN(V, AST) auto V = AST; if (!V) return nullptr

	m_Compiler->InsertLineCount(m_Location.Line);

	switch (m_OperatorToken)
	{
		case HazeToken::Add:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return BINARYOPER(Add);
		}
		case HazeToken::Sub:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return BINARYOPER(Sub);
		}
		case HazeToken::Mul:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return BINARYOPER(Mul);
		}
		case HazeToken::Div:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return BINARYOPER(Div);
		}
		case HazeToken::Inc:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateInc(left, false);
		}
		case HazeToken::Dec:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateDec(left, false);
		}
		case HazeToken::Mod:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return m_Compiler->CreateMod(nullptr, left, right);
		}
		case HazeToken::Not:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return m_Compiler->CreateNot(left, right);
		}
		case HazeToken::Shl:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return BINARYOPER(Shl);
		}
		case HazeToken::Shr:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return BINARYOPER(Shr);
		}
		case HazeToken::BitAnd:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return BINARYOPER(BitAnd);
		}
		case HazeToken::BitOr:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return BINARYOPER(BitOr);
		}
		case HazeToken::BitXor:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			return BINARYOPER(BitXor);
		}
		case HazeToken::Assign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateMov(left, right);
		}
		case HazeToken::AddAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateAdd(left, left, right);
		}
		case HazeToken::SubAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateSub(left, left, right);
		}
		case HazeToken::MulAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateMul(left, left, right);
		}
		case HazeToken::DivAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateDiv(left, left, right);
		}
		case HazeToken::ModAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateMod(left, left, right);
		}
		case HazeToken::BitAndAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateBitAnd(left, left, right);
		}
		case HazeToken::BitOrAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateBitOr(left, left, right);
		}
		case HazeToken::BitXorAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateBitXor(left, left, right);
		}
		case HazeToken::ShlAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateShl(left, left, right);
		}
		case HazeToken::ShrAssign:
		{
			ASSERT_GEN(right, m_RightAST->CodeGen());
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			return m_Compiler->CreateShr(left, left, right);
		}
		case HazeToken::And:
		{
			auto shortBlock = TryShortCircuit(this);
			if (shortBlock)
			{
				m_RightBlock = shortBlock;
			}
			SetChildBlock();
			auto rightExp = dynamic_cast<ASTBinaryExpression*>(m_RightAST.get());
			auto leftExp = dynamic_cast<ASTBinaryExpression*>(m_LeftAST.get());

			if (m_ShortCircuitBlock)
			{
				m_Compiler->SetInsertBlock(m_ShortCircuitBlock->GetShared());
			}

			auto leftValue = m_LeftAST->CodeGen();
			if (leftValue)
			{
				if (!leftExp)
				{
					m_Compiler->CreateBoolCmp(leftValue);
				}
				m_Compiler->CreateCompareJmp(leftExp ? GetHazeCmpTypeByToken(leftExp->m_OperatorToken) : HazeCmpType::Equal, nullptr, m_RightBlock->GetShared());
			}

			if (m_ShortCircuitBlock)
			{
				m_Compiler->SetInsertBlock(m_ShortCircuitBlock->GetShared());
			}

			auto rightValue = m_RightAST->CodeGen();
			if (rightValue)
			{
				if (!rightExp)
				{
					m_Compiler->CreateBoolCmp(rightValue);
				}
				m_Compiler->CreateCompareJmp(rightExp ? GetHazeCmpTypeByToken(rightExp->m_OperatorToken) : HazeCmpType::Equal, m_LeftBlock ? m_LeftBlock->GetShared() : nullptr,
					m_RightBlock->GetShared());

			}

			if (m_ShortCircuitBlock)
			{
				m_Compiler->SetInsertBlock(m_ShortCircuitBlock->GetParentBlock()->GetShared());
			}
			return nullptr;
		}
		case HazeToken::Or:
		{
			SetChildBlock();
			auto rightExp = dynamic_cast<ASTBinaryExpression*>(m_RightAST.get());
			auto leftExp = dynamic_cast<ASTBinaryExpression*>(m_LeftAST.get());
			
			if (m_ShortCircuitBlock)
			{
				m_Compiler->SetInsertBlock(m_ShortCircuitBlock->GetShared());
			}

			auto leftValue = m_LeftAST->CodeGen();
			if (leftValue)
			{
				if (!leftExp)
				{
					m_Compiler->CreateBoolCmp(leftValue);
				}
				m_Compiler->CreateCompareJmp(leftExp ? GetHazeCmpTypeByToken(leftExp->m_OperatorToken) : HazeCmpType::Equal, m_LeftBlock ? m_LeftBlock->GetShared() : nullptr, nullptr);
			}

			if (m_ShortCircuitBlock)
			{
				m_Compiler->SetInsertBlock(m_ShortCircuitBlock->GetShared());
			}
			auto rightValue = m_RightAST->CodeGen();
			if (rightValue)
			{
				if (!rightExp)
				{
					m_Compiler->CreateBoolCmp(rightValue);
				}
				m_Compiler->CreateCompareJmp(rightExp ? GetHazeCmpTypeByToken(rightExp->m_OperatorToken) : HazeCmpType::Equal, m_LeftBlock ? m_LeftBlock->GetShared() : nullptr,
					m_RightBlock ? m_RightBlock->GetShared() : nullptr);
			}

			if (m_ShortCircuitBlock)
			{
				m_Compiler->SetInsertBlock(m_ShortCircuitBlock->GetParentBlock()->GetShared());
			}

			return nullptr;
		}
		case HazeToken::Equal:
		case HazeToken::NotEqual:
		case HazeToken::Greater:
		case HazeToken::GreaterEqual:
		case HazeToken::Less:
		case HazeToken::LessEqual:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen());
			ASSERT_GEN(right, m_RightAST->CodeGen());
			auto retValue = m_Compiler->CreateIntCmp(left, right);
			if (m_LeftBlock || m_RightBlock)
			{
				if (m_ParentAst)
				{
					AST_ERR_W("<%s><%s>�Ƚϴ���, ��������������", m_LeftAST->GetName(), m_RightAST->GetName());
				}

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
	auto m = m_Compiler->ParseModuleByImportPath(m_ModulePath);
	if (m)
	{
		m_Compiler->AddImportModuleToCurrModule(m);
	}
	else
	{
		AST_ERR_W("��<%s>ģ�����", m_ModulePath.c_str());
		m_Compiler->MarkCompilerError();
	}

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
		ConditionExp->SetLeftAndRightBlock(ifThenBlock.get(), m_ElseExpression ? elseBlock.get() : nextBlock.get(), nullptr);
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
		conditionExp->SetLeftAndRightBlock(whileBlock.get(), nextBlock.get(), nullptr);
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
		AST_ERR_W("ѭ������ʼ��ʧ��");
		return nullptr;
	}

	m_Compiler->CreateJmpToBlock(forConditionBlock);
	m_Compiler->SetInsertBlock(forConditionBlock);
	auto conditionExp = dynamic_cast<ASTBinaryExpression*>(m_ConditionExpression.get());
	if (conditionExp)
	{
		conditionExp->SetLeftAndRightBlock(loopBlock.get(), endLoopBlock.get(), nullptr);
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
	auto value = m_Expression->CodeGen();
	if (value)
	{
		return m_Compiler->CreateCast(m_DefineVariable.Type, value);
	}
	else
	{
		AST_ERR_W("ǿת<%s>����", m_Expression->GetName());
	}

	return nullptr;
}

ASTSizeOf::ASTSizeOf(Compiler* compiler, const SourceLocation& location, const HazeDefineType& type, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression))
{
	m_DefineVariable.Type = type;
}

Share<CompilerValue> ASTSizeOf::CodeGen()
{
	if (m_Expression)
	{
		auto value = m_Expression->CodeGen();
		if (value)
		{
			return m_Compiler->GetConstantValueUint64(value->GetSize());
		}
		else
		{
			AST_ERR_W("���<%s>��С����", m_Expression->GetName());
		}
	}
	else
	{
		return m_Compiler->GetConstantValueUint64(m_DefineVariable.Type.GetCompilerTypeSize());
	}

	return nullptr;
}