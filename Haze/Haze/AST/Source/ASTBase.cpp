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
#include "HazeCompilerPointerValue.h"
#include "CompilerClassValue.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"
#include "CompilerHelper.h"

#define CHECK_ASSIGN_VALUE_IS_NULL(AST, V) if(V) AST_ERR_W(#AST H_TEXT("<%s>�����˸���ֵ"), m_DefineVariable.Name.c_str())

//Base
ASTBase::ASTBase(Compiler* compiler, const SourceLocation& location) : m_Compiler(compiler), m_Location(location)
{
	m_DefineVariable.Name = H_TEXT("");
	m_DefineVariable.Type = { HazeValueType::Void };
}

ASTBase::ASTBase(Compiler* compiler, const SourceLocation& location, const HazeDefineVariable& var) 
	: m_Compiler(compiler), m_DefineVariable(var), m_Location(location)
{
	memset(&m_Value.Value, 0, sizeof(m_Value.Value));
}

Share<CompilerValue> ASTBase::TryAssign(Share<CompilerValue> assignTo, Share<CompilerValue> value)
{
	if (assignTo)
	{
		return m_Compiler->CreateMov(assignTo, value);
	}
	return value;
}

ASTBool::ASTBool(Compiler* compiler, const SourceLocation& location, const HazeValue& value)
	: ASTBase(compiler, location)
{
	m_DefineVariable.Type.PrimaryType = HazeValueType::Bool;
	m_Value = value;
}

Share<CompilerValue> ASTBool::CodeGen(Share<CompilerValue> assignTo)
{
	auto v = m_Compiler->GenConstantValue(m_DefineVariable.Type.PrimaryType, m_Value);
	return TryAssign(assignTo, v);
}

ASTNumber::ASTNumber(Compiler* compiler, const SourceLocation& location, HazeValueType type, const HazeValue& value) 
	: ASTBase(compiler, location)
{
	m_DefineVariable.Type.PrimaryType = type;
	m_Value = value;
}

Share<CompilerValue> ASTNumber::CodeGen(Share<CompilerValue> assignTo)
{
	auto v = m_Compiler->GenConstantValue(m_DefineVariable.Type.PrimaryType, m_Value);
	return TryAssign(assignTo, v);
}

ASTStringText::ASTStringText(Compiler* compiler, const SourceLocation& location, HString& text) 
	: ASTBase(compiler, location), m_Text(Move(text))
{
}

Share<CompilerValue> ASTStringText::CodeGen(Share<CompilerValue> assignTo)
{
	auto v = m_Compiler->GenStringVariable(m_Text);
	return TryAssign(assignTo, v);
}

ASTIdentifier::ASTIdentifier(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, HString& name,
	V_Array<Unique<ASTBase>>& arrayIndexExpression, Unique<ASTBase>& preAst, HString nameSpace)
	: ASTBase(compiler, location), m_SectionSignal(section), m_ArrayIndexExpression(Move(arrayIndexExpression)),
	m_PreAst(Move(preAst)), m_NameSpace(Move(nameSpace))
{
	m_DefineVariable.Name = Move(name);
}

Share<CompilerValue> ASTIdentifier::CodeGen(Share<CompilerValue> assignTo)
{
	Share<CompilerValue> retValue = nullptr;
	HazeVariableScope classMemberScope = HazeVariableScope::Local;

	if (!m_DefineVariable.Name.empty())
	{
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
	}
	else
	{
		retValue = m_PreAst->CodeGen();
	}

	if (retValue)
	{
		if (retValue->IsClassMember())
		{
			retValue->SetScope(classMemberScope);
		}

		if (retValue->IsArray() && m_ArrayIndexExpression.size() > 0)
		{
			V_Array<Share<CompilerValue>> indexValue;
			for (size_t i = 0; i < m_ArrayIndexExpression.size(); i++)
			{
				indexValue.push_back(m_ArrayIndexExpression[i]->CodeGen());
			}

			retValue = m_Compiler->CreateArrayElement(retValue, indexValue);
		}
	}
	/*else
	{
		auto function = m_Compiler->GetCurrModule()->GetFunction(m_DefineVariable.Name);
		if (!function.first)
		{
			HAZE_LOG_ERR_W("δ���ҵ�����<%s>,��ǰ����<%s>!\n", m_DefineVariable.Name.c_str(),
				m_SectionSignal == HazeSectionSignal::Local ? m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str() : H_TEXT("None"));
			return nullptr;
		}
	}*/

	return TryAssign(assignTo, retValue);
}

ASTFunctionCall::ASTFunctionCall(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	HString& name, V_Array<Unique<ASTBase>>& functionParam, Unique<ASTBase> classObj, Unique<ASTBase>& preAst)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Name(Move(name)),
	m_FunctionParam(Move(functionParam)), m_ClassObj(Move(classObj)), m_PreAst(Move(preAst))
{
}

Share<CompilerValue> ASTFunctionCall::CodeGen(Share<CompilerValue> assignTo)
{
	m_Compiler->InsertLineCount(m_Location.Line);

	Pair<Share<CompilerFunction>, Share<CompilerValue>> funcs = { nullptr, nullptr };
	if (m_ClassObj)
	{
		funcs.first = DynamicCast<CompilerClassValue>(m_ClassObj->CodeGen())->GetOwnerClass()->FindFunction(m_Name);
	}
	else
	{
		funcs = m_Compiler->GetFunction(m_Name);
	}

	V_Array<Share<CompilerValue>> param;

	for (int i = (int)m_FunctionParam.size() - 1; i >= 0; i--)
	{
		param.push_back(m_FunctionParam[i]->CodeGen());
		if (!param.back())
		{
			HAZE_LOG_ERR_W("����<%s>��<%d>�е���<%s>��<%d>����������!\n", m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str(), m_Location.Line, m_Name.c_str(), i + 1);
			return nullptr;
		}
	}
	
	Share<CompilerValue> ret = nullptr;
	if (funcs.first)
	{
		if (funcs.first->GetClass())
		{
			if (!funcs.second)
			{
				funcs.second = m_Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(TOKEN_THIS);//param.push_back(m_Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(TOKEN_THIS));
			}
		}

		ret = m_Compiler->CreateFunctionCall(funcs.first, param, funcs.second);
	}
	else
	{
		//����ָ��
		auto functionPointer = m_Compiler->GetCurrModule()->GetCurrFunction()->GetLocalVariable(m_Name);
		if (functionPointer)
		{
			ret = m_Compiler->CreateFunctionCall(functionPointer, param);
		}
		else
		{
			ret = m_Compiler->CreateFunctionCall(m_PreAst->CodeGen(), param);
		}
	}

	if (!ret)
	{
		HAZE_LOG_ERR_W("���ɺ�������<%s>����, ��麯������ ( �����Ƿ��뺯������ͬһ��\n", m_Name.c_str());
	}

	return TryAssign(assignTo, ret);
}

ASTClassAttr::ASTClassAttr(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	HString& classObjName, Unique<ASTBase>& preAst, HString& attrName, bool isFunction,
	V_Array<Unique<ASTBase>>* functionParam)
	: ASTBase(compiler, location), m_SectionSignal(section), m_PreAst(Move(preAst)), m_IsFunction(isFunction),
	 m_AttrName(Move(attrName))
{
	m_DefineVariable.Name = Move(classObjName);
	if (functionParam)
	{
		m_Params = Move(*functionParam);
	}
}

Share<CompilerValue> ASTClassAttr::CodeGen(Share<CompilerValue> assignTo)
{
	Share<CompilerValue> v;
	if (!m_DefineVariable.Name.empty())
	{
		if (m_SectionSignal == HazeSectionSignal::Global)
		{
			v = m_Compiler->GetGlobalVariable(m_DefineVariable.Name);
		}
		else if (m_SectionSignal == HazeSectionSignal::Local)
		{
			v = m_Compiler->GetLocalVariable(m_DefineVariable.Name);
			if (!v)
			{
				v = m_Compiler->GetGlobalVariable(m_DefineVariable.Name);
			}
		}
	}
	else
	{
		v = m_PreAst->CodeGen();
	}

	auto classValue = DynamicCast<CompilerClassValue>(v);
	Share<CompilerValue> ret = nullptr;
	if (m_IsFunction)
	{
		V_Array<Share<CompilerValue>> param;
		for (int i = (int)m_Params.size() - 1; i >= 0; i--)
		{
			param.push_back(m_Params[i]->CodeGen());
		}	

		if (!classValue && IsAdvanceType(v->GetValueType().PrimaryType))
		{
			ret = m_Compiler->CreateAdvanceTypeFunctionCall(v->GetValueType().PrimaryType, m_AttrName, param,v);
		}
		else
		{
			ret = m_Compiler->CreateFunctionCall(classValue->GetOwnerClass()->FindFunction(m_AttrName), param, classValue);
		}
	}
	else
	{
		ret = classValue->GetMember(m_AttrName);
	}

	return TryAssign(assignTo, ret);
}

ASTVariableDefine::ASTVariableDefine(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	const HazeDefineVariable& defineVar, Unique<ASTBase> expression)
	: ASTBase(compiler, location, defineVar), m_SectionSignal(section), m_Expression(Move(expression))
{
}

struct GlobalVariableInitScope
{
	GlobalVariableInitScope(CompilerModule* m, HazeSectionSignal signal) : Module(m), Signal(signal)
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

	CompilerModule* Module;
	HazeSectionSignal Signal;
};

Share<CompilerValue> ASTVariableDefine::CodeGen(Share<CompilerValue> assignTo)
{
	CHECK_ASSIGN_VALUE_IS_NULL(ASTVariableDefine, assignTo);
	
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	GlobalVariableInitScope scope(currModule.get(), m_SectionSignal);

	auto exprValue = GenExpressionValue();
	auto retValue = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrFunction(),
		m_DefineVariable, m_Location.Line, exprValue);

	if (m_Expression)
	{
		TryAssign(retValue, exprValue);
	}

	return retValue;
}

Share<CompilerValue> ASTVariableDefine::GenExpressionValue()
{
	Share<CompilerValue> exprValue = nullptr;
	if (m_Expression)
	{
		if (auto NullExpression = dynamic_cast<ASTNullPtr*>(m_Expression.get()))
		{
			NullExpression->SetDefineType(m_DefineVariable.Type);
		}

		exprValue = m_Expression->CodeGen();
	}

	return exprValue;
}

ASTVariableDefine_MultiVariable::ASTVariableDefine_MultiVariable(Compiler* compiler, const SourceLocation& location)
	: ASTBase(compiler, location)
{
	m_DefineVariable.Type = HazeValueType::MultiVariable;
	m_DefineVariable.Name = HAZE_MULTI_PARAM_NAME;
}

Share<CompilerValue> ASTVariableDefine_MultiVariable::CodeGen(Share<CompilerValue> assignTo)
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	
	if (assignTo)
	{
		AST_ERR_W("���������<%s>�������, �����˸�ֵ", m_DefineVariable.Name.c_str());
	}
	
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

Share<CompilerValue> ASTVariableDefine_Class::CodeGen(Share<CompilerValue> assignTo)
{
	CHECK_ASSIGN_VALUE_IS_NULL(ASTVariableDefine_Class, assignTo);

	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	GlobalVariableInitScope scope(currModule.get(), m_SectionSignal);

	auto exprValue = GenExpressionValue();
	auto retValue = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrFunction(), 
		m_DefineVariable, m_Location.Line, exprValue);

	if (exprValue)
	{
		TryAssign(retValue, exprValue);
	}

	return retValue;
}

ASTVariableDefine_Array::ASTVariableDefine_Array(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	const HazeDefineVariable& defineVar, Unique<ASTBase> expression, TemplateDefineTypes& templateTypes, V_Array<Unique<ASTBase>>& arraySize)
	: ASTVariableDefine_Class(compiler, location, section, defineVar, Move(expression), templateTypes, Move(arraySize))
{
	if (templateTypes.Types.size() > 0)
	{
		m_TemplateTypes.Types = Move(templateTypes.Types);
	}
}

Share<CompilerValue> ASTVariableDefine_Array::CodeGen(Share<CompilerValue> assignTo)
{
	CHECK_ASSIGN_VALUE_IS_NULL(ASTVariableDefine_Array, assignTo);
	
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	GlobalVariableInitScope scope(currModule.get(), m_SectionSignal);

	auto exprValue = GenExpressionValue();

	V_Array<Share<CompilerValue>> sizeValue;

	for (auto& iter : m_ArraySize)
	{
		auto v = iter->CodeGen();
		if (v->IsConstant())
		{
			sizeValue.push_back(v);
		}
		else
		{
			HAZE_LOG_ERR_W("����<%s>����ʧ�ܣ��������鳤�ȱ����ǳ���! ��ǰ����<%s>\n", m_DefineVariable.Name.c_str(),
				m_SectionSignal == HazeSectionSignal::Local ? currModule->GetCurrFunction()->GetName().c_str() : H_TEXT("None"));
			return nullptr;
		}
	}

	auto retValue = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrFunction(),
		m_DefineVariable, m_Location.Line, exprValue, sizeValue);

	if (exprValue)
	{
		TryAssign(retValue, exprValue);
	}

	return retValue;
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

Share<CompilerValue> ASTVariableDefine_Function::CodeGen(Share<CompilerValue> assignTo)
{
	CHECK_ASSIGN_VALUE_IS_NULL(ASTVariableDefine_Function, assignTo);

	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	GlobalVariableInitScope scope(currModule.get(), m_SectionSignal);

	V_Array<HazeDefineType> paramTypes;
	m_Compiler->GetRealTemplateTypes(m_TemplateTypes, paramTypes);
	auto retValue = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrFunction(),
		m_DefineVariable, m_Location.Line, nullptr, {}, &paramTypes);

	auto exprValue = m_Expression->CodeGen();
	if (!exprValue)
	{
		auto function = m_Compiler->GetCurrModule()->GetFunction(m_Expression->m_DefineVariable.Name);
		if (!function.first)
		{
			HAZE_LOG_ERR_W("δ���ҵ�����<%s>,��ǰ����<%s>!\n", m_DefineVariable.Name.c_str(),
				m_SectionSignal == HazeSectionSignal::Local ? m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str() : H_TEXT("None"));
			return nullptr;
		}

		m_Compiler->CreatePointerToFunction(function.first, retValue);
	}
	else
	{
		TryAssign(retValue, exprValue);
	}

	return retValue;
}

ASTReturn::ASTReturn(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	:ASTBase(compiler, location), m_Expression(Move(expression))
{
}

Share<CompilerValue> ASTReturn::CodeGen(Share<CompilerValue> assignTo)
{
	CHECK_ASSIGN_VALUE_IS_NULL(ASTVariableDefine_Function, assignTo);
	
	m_Compiler->InsertLineCount(m_Location.Line);
	Share<CompilerValue> retValue = m_Expression ? m_Expression->CodeGen(Compiler::GetRegister(RET_REGISTER))
		: m_Compiler->GetConstantValueInt(0);
	if (m_Expression ? retValue->GetValueType() == m_Compiler->GetCurrModule()->GetCurrFunction()->GetFunctionType() : 
		IsVoidType(m_Compiler->GetCurrModule()->GetCurrFunction()->GetFunctionType().PrimaryType))
	{
		return m_Compiler->CreateRet(retValue);
	}
	else
	{
		AST_ERR_W("����ֵ���ʹ���");
	}

	return nullptr;
}

ASTNew::ASTNew(Compiler* compiler, const SourceLocation& location, const HazeDefineVariable& DefineVar, V_Array<Unique<ASTBase>> countArrayExpression
	, V_Array<Unique<ASTBase>> constructorParam)
	: ASTBase(compiler, location, DefineVar), m_CountArrayExpression(Move(countArrayExpression)),
	m_ConstructorParam(Move(constructorParam))
{
}

Share<CompilerValue> ASTNew::CodeGen(Share<CompilerValue> assignTo)
{
	auto func = m_Compiler->GetCurrModule()->GetCurrFunction();

	V_Array<Share<CompilerValue>> countValue(m_CountArrayExpression.size());

	for (uint64 i = 0; i < m_CountArrayExpression.size(); i++)
	{
		countValue[i] = m_CountArrayExpression[i]->CodeGen();
	}

	auto value = m_Compiler->CreateNew(func, m_DefineVariable.Type, &countValue);
	

	//new�����ڴ��������Ļ�����Ҫ���ù��캯��
	if (value->GetValueType().NeedCustomName())
	{
		auto newClass = m_Compiler->GetCurrModule()->GetClass(*value->GetValueType().CustomName);
		if (countValue.size() > 0)
		{
			//��Ҫ��ʼ������������Ĳ�����������Ϊ0
			auto function = newClass->FindFunction(*value->GetValueType().CustomName);
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
			params.push_back(m_Compiler->CreatePointerToFunction(function, nullptr));
			params.push_back(value);

			m_Compiler->CreateFunctionCall(arrayConstructorFunc.first, params);
		}
		else
		{
			auto function = newClass->FindFunction(*value->GetValueType().CustomName);
			V_Array<Share<CompilerValue>> param;

			//�������
			for (int i = (int)m_ConstructorParam.size() - 1; i >= 0; i--)
			{
				param.push_back(m_ConstructorParam[i]->CodeGen());
			}

			value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(value->GetValueType()), value);
			m_Compiler->CreateFunctionCall(function, param, value);
		}
	}


	return TryAssign(assignTo, value);
}

ASTGetAddress::ASTGetAddress(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression))
{
}

ASTGetAddress::~ASTGetAddress()
{
}

Share<CompilerValue> ASTGetAddress::CodeGen(Share<CompilerValue> assignTo)
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
		if (function.first)
		{
			retValue = m_Compiler->CreatePointerToFunction(function.first, nullptr);
		}
		else
		{
			HAZE_LOG_ERR_W("δ���ҵ�����<%s>ȥ��ú�����ַ!\n", m_Expression->GetName());
		}
	}
	/*else
	{
		retValue = m_Compiler->CreatePointerToValue(retValue);
	}*/

	return TryAssign(assignTo, retValue);
}

ASTPointerValue::ASTPointerValue(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression,
	int level, Unique<ASTBase> assignExpression)
	: ASTBase(compiler, location), m_Expression(Move(expression)), m_Level(level),
	m_AssignExpression(Move(assignExpression))
{
}

ASTPointerValue::~ASTPointerValue()
{
}

Share<CompilerValue> ASTPointerValue::CodeGen(Share<CompilerValue> assignTo)
{
	auto pointerValue = m_Expression->CodeGen();

	if (pointerValue)
	{
		if (m_AssignExpression)
		{
			return m_Compiler->CreateMovToPV(pointerValue, m_AssignExpression->CodeGen());
		}

		return m_Compiler->CreateMovPV(m_Compiler->GetTempRegister(pointerValue->GetValueType()), pointerValue);
	}
	else
	{
		HAZE_LOG_ERR_W("δ�ܻ��<%s>ָ��ָ���ֵ!\n", m_DefineVariable.Name.c_str());
	}

	return nullptr;
}

ASTNeg::ASTNeg(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isNumberNeg)
	: ASTBase(compiler, location), m_Expression(Move(expression)), m_IsNumberNeg(isNumberNeg)
{
}

ASTNeg::~ASTNeg()
{
}

Share<CompilerValue> ASTNeg::CodeGen(Share<CompilerValue> assignTo)
{
	Share<CompilerValue> ret = nullptr;
	if (m_IsNumberNeg)
	{
		ret = m_Compiler->CreateNeg(assignTo, m_Expression->CodeGen());
	}
	else
	{
		ret = m_Compiler->CreateBitNeg(assignTo, m_Expression->CodeGen());
	}
	
	return ret;
}

ASTNullPtr::ASTNullPtr(Compiler* compiler, const SourceLocation& location) : ASTBase(compiler, location)
{
}

ASTNullPtr::~ASTNullPtr()
{
}

Share<CompilerValue> ASTNullPtr::CodeGen(Share<CompilerValue> assignTo)
{
	return TryAssign(assignTo, m_Compiler->GetNullPtr(m_DefineVariable.Type));
}

void ASTNullPtr::SetDefineType(const HazeDefineType& type)
{
	m_DefineVariable.Type = type;
}

ASTNot::ASTNot(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression))
{
}

ASTNot::~ASTNot()
{
}

Share<CompilerValue> ASTNot::CodeGen(Share<CompilerValue> assignTo)
{
	auto value = m_Expression->CodeGen();
	return m_Compiler->CreateNot(assignTo, value);
}


ASTInc::ASTInc(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreInc)
	: ASTBase(compiler, location), m_IsPreInc(isPreInc), m_Expression(Move(expression))
{
}

ASTInc::~ASTInc()
{
}

Share<CompilerValue> ASTInc::CodeGen(Share<CompilerValue> assignTo)
{
	m_Compiler->InsertLineCount(m_Location.Line);
	return m_Compiler->CreateInc(m_Expression->CodeGen(), m_IsPreInc);
}

ASTDec::ASTDec(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreDec)
	: ASTBase(compiler, location), m_IsPreDec(isPreDec), m_Expression(Move(expression))
{
}

ASTDec::~ASTDec()
{
}

Share<CompilerValue> ASTDec::CodeGen(Share<CompilerValue> assignTo)
{
	m_Compiler->InsertLineCount(m_Location.Line);
	return m_Compiler->CreateDec(m_Expression->CodeGen(), m_IsPreDec);
}


ASTDataSection::ASTDataSection(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, V_Array<Unique<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(Move(expressions))
{
}

ASTDataSection::~ASTDataSection()
{
}

Share<CompilerValue> ASTDataSection::CodeGen(Share<CompilerValue> assignTo)
{
	for (size_t i = 0; i < m_Expressions.size(); i++)
	{
		m_Expressions[i]->CodeGen();
	}

	return nullptr;
}

ASTMultiExpression::ASTMultiExpression(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	V_Array<Unique<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(Move(expressions))
{
}

ASTMultiExpression::~ASTMultiExpression()
{
}

Share<CompilerValue> ASTMultiExpression::CodeGen(Share<CompilerValue> assignTo)
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
	}

	return nullptr;
}

ASTBinaryExpression::ASTBinaryExpression(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
	HazeToken operatorToken, Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST)
	:ASTBase(compiler, location), m_SectionSignal(section), m_OperatorToken(operatorToken), m_LeftAST(Move(leftAST)),
	m_RightAST(Move(rightAST)), m_LeftBlock(nullptr), m_RightBlock(nullptr), m_DefaultBlock(nullptr)
{
}

ASTBinaryExpression::~ASTBinaryExpression()
{
}

void ASTBinaryExpression::SetLeftAndRightBlock(CompilerBlock* leftJmpBlock, CompilerBlock* rightJmpBlock)
{
	m_LeftBlock = leftJmpBlock;
	m_RightBlock = rightJmpBlock;
}

Share<CompilerValue> ASTBinaryExpression::CodeGen(Share<CompilerValue> assignTo)
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
	{
		auto left = m_LeftAST->CodeGen();
		auto right = m_RightAST->CodeGen();
		return m_Compiler->CreateAdd(assignTo, left, right);
	}
	case HazeToken::Sub:
	{
		auto left = m_LeftAST->CodeGen();
		auto right = m_RightAST->CodeGen();
		return m_Compiler->CreateSub(assignTo, left, right);
	}
	case HazeToken::Mul:
	{
		auto left = m_LeftAST->CodeGen();
		auto right = m_RightAST->CodeGen();
		return m_Compiler->CreateMul(assignTo, left, right);
	}
	case HazeToken::Div:
	{
		auto left = m_LeftAST->CodeGen();
		auto right = m_RightAST->CodeGen();
		return m_Compiler->CreateDiv(assignTo, left, right);
	}
	case HazeToken::Inc:
		return m_Compiler->CreateInc(m_LeftAST->CodeGen(), false);
	case HazeToken::Dec:
		return m_Compiler->CreateDec(m_LeftAST->CodeGen(), false);
	case HazeToken::Mod:
	{
		auto left = m_LeftAST->CodeGen();
		auto right = m_RightAST->CodeGen();
		return m_Compiler->CreateMod(assignTo, left, right);
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
		return m_Compiler->CreateShl(assignTo, left, right);
	}
	case HazeToken::Shr:
	{
		auto left = m_LeftAST->CodeGen();
		auto right = m_RightAST->CodeGen();
		return m_Compiler->CreateShr(assignTo, left, right);
	}
	case HazeToken::Assign:
	{
		auto left = m_LeftAST->CodeGen();
		m_RightAST->CodeGen(left);
		return left;
	}
	case HazeToken::BitAnd:
	{
		auto left = m_LeftAST->CodeGen();
		auto right = m_RightAST->CodeGen();
		return m_Compiler->CreateBitAnd(assignTo, left, right);
	}
	case HazeToken::BitOr:
	{
		auto left = m_LeftAST->CodeGen();
		auto right = m_RightAST->CodeGen();
		return m_Compiler->CreateBitOr(assignTo, left, right);
	}
	case HazeToken::BitXor:
	{
		auto left = m_LeftAST->CodeGen();
		auto right = m_RightAST->CodeGen();
		return m_Compiler->CreateBitXor(assignTo, left, right);
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
	: ASTBase(compiler, location), m_ConditionAST(Move(conditionAST)), m_LeftAST(Move(leftAST)),
		m_RightAST(Move(rightAST))
{
}

ASTThreeExpression::~ASTThreeExpression()
{
}

Share<CompilerValue> ASTThreeExpression::CodeGen(Share<CompilerValue> assignTo)
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

	auto leftValue = m_LeftAST->CodeGen();
	auto tempValue = assignTo;//m_Compiler->GetTempRegister(leftValue->GetValueType());
	m_Compiler->SetInsertBlock(blockLeft);
	m_Compiler->CreateMov(tempValue, leftValue);
	m_Compiler->CreateJmpToBlock(defauleBlock);

	m_Compiler->SetInsertBlock(blockRight);
	m_Compiler->CreateMov(tempValue, m_RightAST->CodeGen());
	m_Compiler->CreateJmpToBlock(defauleBlock);

	m_Compiler->SetInsertBlock(defauleBlock);

	return tempValue;
}

ASTImportModule::ASTImportModule(Compiler* compiler, const SourceLocation& location, const HString& modulepath)
	: ASTBase(compiler, location), m_ModulePath(modulepath)
{
}

ASTImportModule::~ASTImportModule()
{
}

Share<CompilerValue> ASTImportModule::CodeGen(Share<CompilerValue> assignTo)
{
	m_Compiler->AddImportModuleToCurrModule(m_Compiler->ParseModule(m_ModulePath));
	return nullptr;
}

ASTBreakExpression::ASTBreakExpression(Compiler* compiler, const SourceLocation& location) 
	: ASTBase(compiler, location)
{
}

ASTBreakExpression::~ASTBreakExpression()
{
}

Share<CompilerValue> ASTBreakExpression::CodeGen(Share<CompilerValue> assignTo)
{
	m_Compiler->InsertLineCount(m_Location.Line);
	m_Compiler->CreateJmpToBlock(m_Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopEnd()->GetShared());
	return nullptr;
}

ASTContinueExpression::ASTContinueExpression(Compiler* compiler, const SourceLocation& location)
	: ASTBase(compiler, location)
{
}

ASTContinueExpression::~ASTContinueExpression()
{
}

Share<CompilerValue> ASTContinueExpression::CodeGen(Share<CompilerValue> assignTo)
{
	m_Compiler->InsertLineCount(m_Location.Line);
	m_Compiler->CreateJmpToBlock(m_Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopStep()->GetShared());
	return nullptr;
}

ASTIfExpression::ASTIfExpression(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& condition,
	Unique<ASTBase>& ifExpression, Unique<ASTBase>& elseExpression)
	: ASTBase(compiler, location),m_Condition(Move(condition)), m_IfExpression(Move(ifExpression)),
		m_ElseExpression(Move(elseExpression))
{
}

ASTIfExpression::~ASTIfExpression()
{
}

Share<CompilerValue> ASTIfExpression::CodeGen(Share<CompilerValue> assignTo)
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
	:ASTBase(compiler, location), m_Condition(Move(condition)), m_MultiExpression(Move(multiExpression))
{
}

ASTWhileExpression::~ASTWhileExpression()
{
}

Share<CompilerValue> ASTWhileExpression::CodeGen(Share<CompilerValue> assignTo)
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
		m_StepExpression(Move(stepExpression)), m_MultiExpression(Move(multiExpression))
{
}

ASTForExpression::~ASTForExpression()
{
}

Share<CompilerValue> ASTForExpression::CodeGen(Share<CompilerValue> assignTo)
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
		HAZE_LOG_ERR_W("ѭ������ʼ��ʧ��!\n");
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

ASTCast::~ASTCast()
{
}

Share<CompilerValue> ASTCast::CodeGen(Share<CompilerValue> assignTo)
{
	return TryAssign(assignTo, m_Compiler->CreateCast(m_DefineVariable.Type, m_Expression->CodeGen()));
}

ASTSizeOf::ASTSizeOf(Compiler* compiler, const SourceLocation& location, const HazeDefineType& type, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression))
{
	m_DefineVariable.Type = type;
}

ASTSizeOf::~ASTSizeOf()
{
}

Share<CompilerValue> ASTSizeOf::CodeGen(Share<CompilerValue> assignTo)
{
	Share<CompilerValue> ret = nullptr;
	if (m_Expression)
	{
		ret = m_Compiler->GetConstantValueUint64(m_Expression->CodeGen()->GetSize());
	}
	else
	{
		ret = m_Compiler->GetConstantValueUint64(m_DefineVariable.Type.GetTypeSize());
	}

	return TryAssign(assignTo, ret);
}
