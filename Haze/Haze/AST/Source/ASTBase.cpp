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
#include "CompilerPointerFunction.h"
#include "CompilerClassValue.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"
#include "CompilerHelper.h"

#if HAZE_DEBUGGER 
	#define UPDATE_COMPILER_LINE_COUNT() m_Compiler->UpdateLineCount(m_Location.Line)
	#define UPDATE_COMPILER_LINE_COUNT1(LINE) m_Compiler->UpdateLineCount(LINE)
#else
	#define UPDATE_COMPILER_LINE_COUNT()
	#define UPDATE_COMPILER_LINE_COUNT1(LINE)
#endif

struct ASTFunctionCallScope
{
	ASTFunctionCallScope(Compiler* compiler, const STDString* nameSpace = nullptr)
		: C(compiler), NameSpace(nameSpace) { }

	/*void SetThis(Share<CompilerValue> thisPointer)
	{
		if (!This)
		{
			HAZE_LOG_ERR_W("<语法分析错误>：" H_TEXT(" 【<%s>模块】!\n"), C->GetCurrModuleName().c_str());
			C->MarkCompilerError();
		}

		This = thisPointer;
	}*/

	// Left(index 0) -> Right
	void AddParam(ASTBase* paramAST, Share<CompilerValue> inferValue) { Param.push_back(paramAST); InferValue.push_back(inferValue); }

	Share<CompilerValue> Call(Share<CompilerFunction> function, Share<CompilerValue> thisPointer = nullptr)
	{
		for (int i = (int)Param.size() - 1; i >= 0; i--)
		{
			InferValue[i] = Param[i]->CodeGen(InferValue[i]);
			C->CreatePush(InferValue[i]);
		}

		std::reverse(InferValue.begin(), InferValue.end());
		return C->CreateFunctionCall(function, InferValue, false, thisPointer, NameSpace);
	}

	Share<CompilerValue> AdvanceCall(HazeValueType type, const STDString& functionName, Share<CompilerValue> thisPointer = nullptr)
	{ 
		return AdvanceCall(type, functionName.c_str(), thisPointer);
	}

	Share<CompilerValue> AdvanceCall(HazeValueType type, const x_HChar* functionName, Share<CompilerValue> thisPointer = nullptr)
	{
		assert(IsAdvanceType(type));

		for (int i = (int)Param.size() - 1; i >= 0; i--)
		{
			InferValue[i] = Param[i]->CodeGen(InferValue[i]);
			C->CreatePush(InferValue[i]);
		}

		return C->CreateAdvanceTypeFunctionCall(type, functionName, InferValue, thisPointer, false);
	}

private:
	const STDString* NameSpace;
	Compiler* C;
	V_Array<ASTBase*> Param;
	V_Array<Share<CompilerValue>> InferValue;
};

//Base
ASTBase::ASTBase(Compiler* compiler, const SourceLocation& location) : m_Compiler(compiler), m_Location(location)
{
	m_Value.Value.UInt64 = 0;
	m_DefineVariable.Name = H_TEXT("");
}

ASTBase::ASTBase(Compiler* compiler, const SourceLocation& location, HazeDefineVariable&& var)
	: m_Compiler(compiler), m_DefineVariable(Move(var)), m_Location(location)
{
	memset(&m_Value.Value, 0, sizeof(m_Value.Value));
}

ASTBool::ASTBool(Compiler* compiler, const SourceLocation& location, const HazeValue& value) : ASTBase(compiler, location)
{
	m_DefineVariable.Type.SetBaseTypeAndId(HazeValueType::Bool);
	m_Value = value;
}

Share<CompilerValue> ASTBool::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();
	return m_Compiler->GenConstantValue(HazeValueType::Bool, m_Value);
}

ASTNumber::ASTNumber(Compiler* compiler, const SourceLocation& location, HazeValueType type, const HazeValue& value)
	: ASTBase(compiler, location)
{
	m_DefineVariable.Type.SetBaseTypeAndId(type);
	m_Value = value;
}

Share<CompilerValue> ASTNumber::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();
	if (inferValue && m_DefineVariable.Type.BaseType != inferValue->GetBaseType() && !IsMultiVariableType(inferValue->GetBaseType()))
	{
		AST_ERR_W("数字<%s>变量类型与赋值变量不符", HazeValueNumberToString(m_DefineVariable.Type.BaseType, m_Value).c_str());
	}

	return m_Compiler->GenConstantValue(m_DefineVariable.Type.BaseType, m_Value);
}

ASTStringText::ASTStringText(Compiler* compiler, const SourceLocation& location, STDString& text)
	: ASTBase(compiler, location), m_Text(Move(text)) {
}

Share<CompilerValue> ASTStringText::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();
	return m_Compiler->GenStringVariable(m_Text);
}

ASTIdentifier::ASTIdentifier(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, STDString&& name,
	Unique<ASTBase> arrayIndexExpression, Unique<ASTBase> preAst, STDString nameSpace)
	: ASTBase(compiler, location), m_SectionSignal(section), m_ArrayIndexExpression(Move(arrayIndexExpression)),
	m_PreAst(Move(preAst)), m_NameSpace(Move(nameSpace))
{
	m_DefineVariable.Name = Move(name);
}

Share<CompilerValue> ASTIdentifier::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	Share<CompilerValue> preValue = m_PreAst ? m_PreAst->CodeGen(nullptr) : nullptr;
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
					m_ArrayIndexExpression->CodeGen(nullptr));
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
		if (retValue)
		{
			retValue = m_Compiler->CreateElementValue(retValue->IsElement() ? DynamicCast<CompilerElementValue>(retValue)->CreateGetFunctionCall() : retValue,
				m_ArrayIndexExpression->CodeGen(nullptr));
		}
		else
		{
			AST_ERR_W("变量<%s>未能找到", m_DefineVariable.Name.c_str());
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
				retValue = m_Compiler->GenConstantValue((HazeValueType)m_Compiler->GetCurrModule()->GetCurrEnum()->GetTypeId(), retValue->GetValue());
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
	STDString&& name, V_Array<Unique<ASTBase>>& functionParam, Unique<ASTBase> classObj, STDString nameSpaces)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Name(Move(name)),
	m_FunctionParam(Move(functionParam)), m_ClassObj(Move(classObj)), m_NameSpace(Move(nameSpaces)) {
}

Share<CompilerValue> ASTFunctionCall::CodeGen(Share<CompilerValue> inferValue)
{
	auto& currModule = m_Compiler->GetCurrModule();
	V_Array<Share<CompilerValue>> paramInferValues(m_FunctionParam.size());

	Share<CompilerFunction> function = nullptr;
	Share<CompilerValue> closure = nullptr;

	Share<CompilerValue> classObj = nullptr;

	if (m_ClassObj)
	{
		classObj = m_ClassObj->CodeGen(nullptr);
		auto classValue = DynamicCast<CompilerClassValue>(classObj);
		if (classValue)
		{
			function = classValue->GetOwnerClass()->FindFunction(m_Name, m_NameSpace.empty() ? nullptr : &m_NameSpace);
		}
		else if (!classObj)
		{
			AST_ERR_W("未能正确找到对象<%s>", m_ClassObj->GetName());
		}
	}
	else
	{
		function = m_Compiler->GetFunction(m_Name);
		if (!function)
		{
			closure = currModule->GetClosureVariable(m_Name, true);
			if (!closure)
			{
				closure = currModule->GetCurrFunction()->GetLocalVariable(m_Name, m_NameSpace.empty() ? nullptr : &m_NameSpace);
			}
		}
	}

	AdvanceFunctionInfo* advanceFunctionInfo = nullptr;
	if (function)
	{
		for (x_uint64 i = 0; i < paramInferValues.size(); i++)
		{
			paramInferValues[i] = function->GetParamVariableLeftToRight((x_uint32)i);
		}
	}
	else if (closure)
	{
		auto pointerFuncValue = DynamicCast<CompilerPointerFunction>(closure);
		for (x_uint64 i = 0; i < paramInferValues.size(); i++)
		{
			paramInferValues[i] = CreateAstTempVariable(currModule.get(), pointerFuncValue->GetParamTypeLeftToRightByIndex((x_uint32)i));
		}
	}
	else if (classObj)
	{
		advanceFunctionInfo = m_Compiler->GetAdvanceFunctionInfo(classObj->GetBaseType(), m_Name);
		if (advanceFunctionInfo)
		{
			for (x_uint64 i = 0; i < paramInferValues.size(); i++)
			{
				paramInferValues[i] = CreateAstTempVariable(currModule.get(), advanceFunctionInfo->Params[i]);
			}
		}
		else if (IsDynamicClassType(classObj->GetBaseType())) { }
		else
		{
			AST_ERR_W("生成函数调用错误, 未能找到函数<%s>", m_Name.c_str());
		}
	}


	ASTFunctionCallScope callScope(m_Compiler, m_NameSpace.empty() ? nullptr : &m_NameSpace);


	for (x_uint64 i = 0; i < m_FunctionParam.size(); i++)
	{
		callScope.AddParam(m_FunctionParam[i].get(), paramInferValues[i]);
	}

	/*for (int i = (int)m_FunctionParam.size() - 1; i >= 0; i--)
	{
		auto value = m_FunctionParam[i]->CodeGen(paramInferValues[i]);
		m_Compiler->CreatePush(value);

		paramInferValues[i] = value;
		if (!value)
		{
			AST_ERR_W("函数<%s>中<%d>行调用<%s>第<%d>个参数错误", currModule->GetCurrFunction()->GetName().c_str(), m_Location.Line, m_Name.c_str(), i + 1);
			return nullptr;
		}
	}*/

	UPDATE_COMPILER_LINE_COUNT();

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

			ret = callScope.Call(function, classObj);

			//ret = m_Compiler->CreateFunctionCall(function, paramInferValues, classObj, m_NameSpace.empty() ? nullptr : &m_NameSpace);
		}
		/*else
		{
			ret = m_Compiler->CreateFunctionCall(function, paramInferValues, classObj, m_NameSpace.empty() ? nullptr : &m_NameSpace);
		}*/

		ret = callScope.Call(function, classObj);
	}
	else if (closure)
	{
		ret = callScope.AdvanceCall(HazeValueType::Closure, HAZE_CUSTOM_CALL_FUNCTION, closure);
		//ret = m_Compiler->CreateAdvanceTypeFunctionCall(HazeValueType::Closure, HAZE_CUSTOM_CALL_FUNCTION, paramInferValues, pointerFunction, false);
	}
	else if (classObj)
	{
		if (classObj->IsElement())
		{
			auto elementValue = DynamicCast<CompilerElementValue>(classObj);
			ret = callScope.AdvanceCall(elementValue->GetParentBaseType().BaseType, m_Name, classObj);
			//ret = m_Compiler->CreateAdvanceTypeFunctionCall(elementValue->GetParentBaseType().BaseType, m_Name, paramInferValues, elementValue, false);
		}
		/*else if (classObj->IsDynamicClass())
		{
			ret = m_Compiler->CreateAdvanceTypeFunctionCall(HazeValueType::DynamicClass, m_Name, paramInferValues, classObj, false);
		}
		else if (classObj->IsObjectBase())
		{
			ret = m_Compiler->CreateAdvanceTypeFunctionCall(HazeValueType::ObjectBase, m_Name, paramInferValues, classObj, false);
		}
		else if (classObj->IsClass())
		{
			ret = m_Compiler->CreateAdvanceTypeFunctionCall(HazeValueType::Class, m_Name, paramInferValues, classObj, false);
		}*/
		else
		{
			ret = callScope.AdvanceCall(classObj->GetBaseType(), m_Name, classObj);
			//AST_ERR_W("生成函数调用错误, <%s>类型错误", m_Name.c_str());
		}
	}


	if (!ret && inferValue)
	{
		AST_ERR_W("生成函数调用<%s>错误, 检查函数名或 ( 符号是否与函数名在同一行", m_Name.c_str());
	}

	if (inferValue)
	{
		if (!IsMultiVariableType(inferValue->GetBaseType()))
		{
			ret = m_Compiler->CreateFunctionRet(inferValue->GetVariableType());
		}
		else
		{
			ret = m_Compiler->CreateFunctionRet(ret->GetVariableType());
		}
	}

	return ret;
}

ASTVariableDefine::ASTVariableDefine(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	HazeDefineVariable&& defineVar, Unique<ASTBase> expression)
	: ASTBase(compiler, location, Move(defineVar)), m_SectionSignal(section), m_Expression(Move(expression))
{
	if (m_DefineVariable.Name.empty())
	{
		PARSE_AST_ERR_W("变量定义错误, 未能获得名称");
	}
}

Share<CompilerValue> ASTVariableDefine::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);
	return TryAssign(var, H_TEXT("变量"));
}

Share<CompilerValue> ASTVariableDefine::GenExpressionValue(Share<CompilerValue> value)
{
	Share<CompilerValue> exprValue = nullptr;
	/*if (auto nullExpression = dynamic_cast<ASTNullPtr*>(m_Expression.get()))
	{
		nullExpression->SetDefineType(m_DefineVariable.Type);
	}*/

	exprValue = m_Expression->CodeGen(value);
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

Share<CompilerValue> ASTVariableDefine::TryAssign(Share<CompilerValue> var, const x_HChar* errorPrefix)
{
	if (var)
	{
		if (m_Expression)
		{
			if (m_Compiler->GetCurrModule()->IsBeginCreateFunctionParamVariable())
			{
				m_Compiler->GetCurrModule()->GetCurrClosureOrFunction()->SetParamDefaultAST(m_Expression);
				return var;
			}
			else if (m_Compiler->GetCurrModule()->IsBeginCreateClassVariable())
			{
				return var;
			}
			else
			{
				auto v = GenExpressionValue(var);
				if (v)
				{
					UPDATE_COMPILER_LINE_COUNT();
					return m_Compiler->CreateMov(var, v);
				}
				else
				{
					AST_ERR_W("%s<%s>定义赋值右边表达式错误", errorPrefix, m_DefineVariable.Name.c_str());
				}
			}
		}
		else
		{
			return var;
		}
	}
	else
	{
		AST_ERR_W("%s<%s>定义错误", errorPrefix, m_DefineVariable.Name.c_str());
	}

	return nullptr;
}

ASTVariableDefine_MultiVariable::ASTVariableDefine_MultiVariable(Compiler* compiler, const SourceLocation& location)
	: ASTBase(compiler, location)
{
	m_DefineVariable.Type = HazeVariableType(HazeValueType::MultiVariable, HAZE_TYPE_ID(HazeValueType::MultiVariable));
	m_DefineVariable.Name = HAZE_MULTI_PARAM_NAME;
}

Share<CompilerValue> ASTVariableDefine_MultiVariable::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	return m_Compiler->CreateVariableBySection(HazeSectionSignal::Local, currModule, currModule->GetCurrClosureOrFunction(), m_DefineVariable, m_Location.Line);
}

ASTVariableDefine_Class::ASTVariableDefine_Class(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	HazeDefineVariable&& defineVar, Unique<ASTBase> expression, V_Array<Unique<ASTBase>> params)
	: ASTVariableDefine(compiler, location, section, Move(defineVar), Move(expression)), m_Params(Move(params))
{
}

Share<CompilerValue> ASTVariableDefine_Class::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);
	return TryAssign(var, H_TEXT("类变量"));
}

ASTVariableDefine_Array::ASTVariableDefine_Array(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	HazeDefineVariable&& defineVar, Unique<ASTBase> expression)
	: ASTVariableDefine(compiler, location, section, Move(defineVar), Move(expression))/*, m_ArrayDimension(dimension)*/
{
	/*if (templateTypes.Types.size() > 0)
	{
		m_TemplateTypes.Types = Move(templateTypes.Types);
	}*/
}

Share<CompilerValue> ASTVariableDefine_Array::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);

	return TryAssign(var, H_TEXT("数组变量"));
}

ASTVariableDefine_Function::ASTVariableDefine_Function(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	HazeDefineVariable&& defineVar, Unique<ASTBase> expression, x_uint32 templateTypeId)
	: ASTVariableDefine(compiler, location, section, Move(defineVar), Move(expression)), m_TemplateTypeId(templateTypeId)
{
}

Share<CompilerValue> ASTVariableDefine_Function::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	//V_Array<HazeVariableType> paramTypes(m_TemplateTypes.Types.size() + 1);

	////返回类型设置到第0个
	//paramTypes[0] = m_DefineVariable.Type;
	//m_Compiler->GetRealTemplateTypes(m_TemplateTypes, paramTypes);

	/*for (x_uint64 i = 0; i < m_TemplateTypes.Types.size(); i++)
	{
		paramTypes[i + 1] = *m_TemplateTypes.Types[i].Type;
	}*/

	m_DefineVariable.Type.BaseType = HazeValueType::Function;
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);

	return TryAssign(var, H_TEXT("函数指针变量"));
}

ASTVariableDefine_ObjectBase::ASTVariableDefine_ObjectBase(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	HazeDefineVariable&& defineVar, Unique<ASTBase> expression)
	: ASTVariableDefine(compiler, location, section, Move(defineVar), Move(expression))
{
}

Share<CompilerValue> ASTVariableDefine_ObjectBase::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);

	return TryAssign(var, H_TEXT("基本对象变量"));
}

ASTVariableDefine_Hash::ASTVariableDefine_Hash(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	HazeDefineVariable&& defineVar, Unique<ASTBase> expression, x_uint32 templateTypeId)
	: ASTVariableDefine(compiler, location, section, Move(defineVar), Move(expression)), m_TemplateTypeId(templateTypeId)
{
}

Share<CompilerValue> ASTVariableDefine_Hash::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);

	return TryAssign(var, H_TEXT("函数指针变量"));
}

ASTVariableDefine_Closure::ASTVariableDefine_Closure(Compiler* compiler, const SourceLocation& location, const SourceLocation& startLocation, const SourceLocation& endLocation, HazeSectionSignal section,
	HazeDefineVariable&& defineVar, Unique<ASTBase>& expression, x_uint32 templateTypeId, V_Array<Unique<ASTBase>>& params)
	: ASTVariableDefine_Function(compiler, location, section, Move(defineVar), Move(expression), templateTypeId), m_Params(Move(params)), m_StartLocation(startLocation), m_EndLocation(endLocation)
{
}

Share<CompilerValue> ASTVariableDefine_Closure::CodeGen(Share<CompilerValue> inferValue)
{
	Unique<CompilerModule>& currModule = m_Compiler->GetCurrModule();

	Share<CompilerClosureFunction> closureFunction = nullptr;
	Share<CompilerClass> currClass = nullptr;

	V_Array<HazeDefineVariable> paramDefines(m_Params.size());
	for (x_uint64 i = 0; i < m_Params.size(); i++)
	{
		paramDefines[i] = m_Params[i]->GetDefine();
	}

	closureFunction = currModule->CreateClosureFunction(m_DefineVariable.Type, paramDefines);
	closureFunction->SetStartEndLine(m_StartLocation.Line, m_EndLocation.Line);
	m_Compiler->SetInsertBlock(closureFunction->GetEntryBlock());

	for (int i = (int)m_Params.size() - 1; i >= 0; i--)
	{
		//currModule->BeginCreateFunctionParamVariable();
		m_Params[i]->CodeGen(nullptr);
		//currModule->EndCreateFunctionParamVariable();
	}

	if (m_Expression)
	{
		m_Expression->CodeGen(nullptr);
	}

	if (closureFunction == currModule->GetCurrClosure())
	{
		currModule->FinishClosure();
	}
	else
	{
		//auto& m_Location = m_EndLocation;
		AST_ERR_W("生成匿名函数<%s>结束错误, 不是当前模块解析的函数<%s>", m_DefineVariable.Name.c_str(), currModule->GetCurrClosure()->GetName().c_str());
	}

	UPDATE_COMPILER_LINE_COUNT();
	auto closureValue = m_Compiler->CreateNew(HazeVariableType(HazeValueType::Closure, m_TemplateTypeId), nullptr, closureFunction);
	//V_Array<HazeVariableType> paramTypes(m_TemplateTypes.Types.size() + 1);

	////返回类型设置到第0个
	//m_TemplateTypes.Types.push_back({ false, nullptr, MakeShare<HazeNewDefineType>(m_DefineVariable.Type) });
	////m_Compiler->GetRealTemplateTypes(m_TemplateTypes, paramTypes);

	//for (x_uint64 i = 0; i < m_Params.size(); i++)
	//{
	//	paramTypes[i + 1] = m_Params[i]->GetDefine().Type;
	//}

	m_DefineVariable.Type = HazeVariableType(HazeValueType::Closure, m_TemplateTypeId);
	auto var = m_Compiler->CreateVariableBySection(m_SectionSignal, currModule, currModule->GetCurrClosureOrFunction(),
		m_DefineVariable, m_Location.Line, nullptr);

	return m_Compiler->CreateMov(var, closureValue);
}

ASTReturn::ASTReturn(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	:ASTBase(compiler, location), m_Expression(Move(expression)) {
}

Share<CompilerValue> ASTReturn::CodeGen(Share<CompilerValue> inferValue)
{
	auto& funcType = m_Compiler->GetCurrModule()->GetCurrClosureOrFunction()->GetFunctionType();
	auto retReg = m_Compiler->GetRetRegister(funcType.BaseType, 0);
	Share<CompilerValue> retValue = m_Expression ? m_Expression->CodeGen(nullptr) : m_Compiler->GetConstantValueInt(0);
	auto retValueType = retValue->GetVariableType();
	/*if (retValue->IsEnum())
	{
		retValueType = DynamicCast<CompilerEnumValue>(retValue)->GetBaseType();
	}*/

	if (m_Expression ? (retValueType == funcType || CanCVT(funcType.BaseType, retValueType.BaseType)) : IsVoidType(funcType.BaseType))
	{
		UPDATE_COMPILER_LINE_COUNT();
		return m_Compiler->CreateRet(retValue);
	}
	else
	{
		AST_ERR_W("返回值类型错误");
	}

	return nullptr;
}

ASTNew::ASTNew(Compiler* compiler, const SourceLocation& location, HazeDefineVariable&& DefineVar/*, TemplateDefineTypes& templateTypes*/, V_Array<Unique<ASTBase>> countArrayExpression
	, V_Array<Unique<ASTBase>> constructorParam)
	: ASTBase(compiler, location, Move(DefineVar)), m_CountArrayExpression(Move(countArrayExpression)),
	m_ConstructorParam(Move(constructorParam))
{
	/*if (templateTypes.Types.size() > 0)
	{
		m_TemplateTypes.Types = Move(templateTypes.Types);
	}*/
}

Share<CompilerValue> ASTNew::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	if (!IsAdvanceType(m_DefineVariable.Type.BaseType) || IsDynamicClassType(m_DefineVariable.Type.BaseType))
	{
		AST_ERR_W("生成类型错误, 只能生成<类><数组><字符串>");
		return nullptr;
	}

	m_DefineVariable.Type.TypeId = inferValue->GetTypeId();

	//auto func = m_Compiler->GetCurrModule()->GetCurrFunction();

	V_Array<Share<CompilerValue>> countValue(m_CountArrayExpression.size());

	for (x_uint64 i = 0; i < m_CountArrayExpression.size(); i++)
	{
		//if (dynamic_cast<ASTNumber*>(m_CountArrayExpression[i].get()))
		{
			countValue[i] = m_CountArrayExpression[i]->CodeGen(nullptr);
		}
		/*else
		{
			AST_ERR_W("生成类型错误, 数组初始化长度只能是常量");
			return nullptr;
		}*/
	}

	Share<CompilerValue> value = m_Compiler->CreateNew(m_DefineVariable.Type, &countValue);

	ASTFunctionCallScope callScope(m_Compiler);

	//new申请内存后，若是类的话，需要调用构造函数
	if (value->IsClass())
	{
		auto classValue = DynamicCast<CompilerClassValue>(value);
		auto newClass = m_Compiler->GetCurrModule()->GetClass(classValue->GetOwnerClassName());
		if (countValue.size() > 0)
		{
			HAZE_TO_DO(类数组调用无参构造函数);

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
			if (newClass->HasDefaultDataAST())
			{
				for (x_uint64 i = 0; i < newClass->m_DefaultValueAST.size(); i++)
				{
					auto member = classValue->GetMember(newClass->m_DefaultValueAST[i].first);
					m_Compiler->CreateSetAdvanceElement(m_Compiler->CreateElementValue(value, member), newClass->m_DefaultValueAST[i].second->CodeGen(member));
				}
			}

			auto function = newClass->FindFunction(classValue->GetOwnerClassName(), nullptr);
			//value = m_Compiler->CreateMov(m_Compiler->GetTempRegister(value->GetVariableType()), value);


			if (function)
			{
				//V_Array<Share<CompilerValue>> param;

				//构造参数
				for (x_uint64 i = 0; i < m_ConstructorParam.size(); i++)
				{
					callScope.AddParam(m_ConstructorParam[i].get(), function->GetParamVariableLeftToRight(0));
					//param.push_back(m_ConstructorParam[i]->CodeGen(function->GetParamVariableRightToLeft((x_uint32)(m_ConstructorParam.size() - 1 - i))));
				}
				//m_Compiler->CreateFunctionCall(function, param, value);
				callScope.Call(function, value);
			}
		}
	}
	else if (value->IsObjectBase())
	{
		if (m_ConstructorParam.size() == 1)
		{
			callScope.AddParam(m_ConstructorParam[0].get(), nullptr);
			callScope.AdvanceCall(value->GetBaseType(), HAZE_OBJECT_BASE_CONSTRUCTOR, value);

			//m_Compiler->CreateAdvanceTypeFunctionCall(HazeValueType::ObjectBase, HAZE_OBJECT_BASE_CONSTRUCTOR, { m_ConstructorParam[0]->CodeGen(nullptr) }, value);

		}
		else
		{
			AST_ERR_W("基本类型对象参数数量只能为1个");
		}
	}

	return value;
}

ASTGetAddress::ASTGetAddress(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression)) {
}

Share<CompilerValue> ASTGetAddress::CodeGen(Share<CompilerValue> inferValue)
{
	return nullptr;
}

ASTNeg::ASTNeg(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isNumberNeg)
	: ASTBase(compiler, location), m_Expression(Move(expression)), m_IsNumberNeg(isNumberNeg) {
}

Share<CompilerValue> ASTNeg::CodeGen(Share<CompilerValue> inferValue)
{
	auto v = m_Expression->CodeGen(nullptr);
	if (v)
	{
		UPDATE_COMPILER_LINE_COUNT();
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
		AST_ERR_W("<%s>取负错误", m_Expression->GetName());
	}

	return nullptr;
}

ASTNullPtr::ASTNullPtr(Compiler* compiler, const SourceLocation& location) : ASTBase(compiler, location) {}

Share<CompilerValue> ASTNullPtr::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();
	return m_Compiler->GetNullPtr();
}

void ASTNullPtr::SetDefineType(const HazeVariableType& type)
{
	m_DefineVariable.Type = type;
}

ASTNot::ASTNot(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression)) {
}

Share<CompilerValue> ASTNot::CodeGen(Share<CompilerValue> inferValue)
{
	auto value = m_Expression->CodeGen(nullptr);
	if (value)
	{
		UPDATE_COMPILER_LINE_COUNT();
		return m_Compiler->CreateNot(nullptr, value);
	}
	else
	{
		AST_ERR_W("<%s>取反错误", m_Expression->GetName());
	}

	return nullptr;
}


ASTInc::ASTInc(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreInc)
	: ASTBase(compiler, location), m_IsPreInc(isPreInc), m_Expression(Move(expression)) {
}

Share<CompilerValue> ASTInc::CodeGen(Share<CompilerValue> inferValue)
{
	auto value = m_Expression->CodeGen(nullptr);
	if (value)
	{
		UPDATE_COMPILER_LINE_COUNT();
		return m_Compiler->CreateInc(value, m_IsPreInc);
	}
	else
	{
		AST_ERR_W("<%s>自加错误", m_Expression->GetName());
	}

	return nullptr;
}

ASTDec::ASTDec(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreDec)
	: ASTBase(compiler, location), m_IsPreDec(isPreDec), m_Expression(Move(expression)) {
}

Share<CompilerValue> ASTDec::CodeGen(Share<CompilerValue> inferValue)
{
	auto value = m_Expression->CodeGen(nullptr);
	if (value)
	{
		UPDATE_COMPILER_LINE_COUNT();
		return m_Compiler->CreateDec(value, m_IsPreDec);

	}
	else
	{
		AST_ERR_W("<%s>自减错误", m_Expression->GetName());
	}

	return nullptr;
}


ASTDataSection::ASTDataSection(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, V_Array<Unique<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(Move(expressions)) {
}

Share<CompilerValue> ASTDataSection::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	m_Compiler->GetCurrModule()->BeginGlobalDataDefine();

	for (size_t i = 0; i < m_Expressions.size(); i++)
	{
		m_Expressions[i]->CodeGen(nullptr);
	}

	m_Compiler->GetCurrModule()->EndGlobalDataDefine();
	return nullptr;
}

ASTMultiExpression::ASTMultiExpression(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
	V_Array<Unique<ASTBase>>& expressions)
	: ASTBase(compiler, location), m_SectionSignal(section), m_Expressions(Move(expressions)) {
}

Share<CompilerValue> ASTMultiExpression::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();
	
	//要考虑清楚无效的临时寄存器中引用的情况，全局变量的初始不用考虑，会在执行进入主函数字节码之前初始化完成
	auto func = m_Compiler->GetCurrModule()->GetCurrFunction();
	if (!func)
	{
		AST_ERR_W("执行多行式AST错误, 未能找打当前运行函数");
	}

	for (x_uint64 i = 0; i < m_Expressions.size(); i++)
	{
		m_Expressions[i]->CodeGen(nullptr);
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
	m_RightAST(Move(rightAST)), m_LeftBlock(nullptr), m_RightBlock(nullptr), m_ParentAst(nullptr), m_ShortCircuitBlock(nullptr), m_AssignToAst(nullptr) {
}

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

Share<CompilerValue> ASTBinaryExpression::CodeGen(Share<CompilerValue> inferValue)
{
#define BINARYOPER(OPER) m_Compiler->Create##OPER(m_AssignToAst ? m_AssignToAst->CodeGen(nullptr) : nullptr, left, right);
#define ASSERT_GEN(V, AST) auto V = AST; if (!V) return nullptr

	switch (m_OperatorToken)
	{
		case HazeToken::Add:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return BINARYOPER(Add);
		}
		case HazeToken::Sub:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return BINARYOPER(Sub);
		}
		case HazeToken::Mul:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return BINARYOPER(Mul);
		}
		case HazeToken::Div:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return BINARYOPER(Div);
		}
		case HazeToken::Inc:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateInc(left, false);
		}
		case HazeToken::Dec:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateDec(left, false);
		}
		case HazeToken::Mod:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateMod(nullptr, left, right);
		}
		case HazeToken::Not:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateNot(left, right);
		}
		case HazeToken::Shl:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return BINARYOPER(Shl);
		}
		case HazeToken::Shr:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return BINARYOPER(Shr);
		}
		case HazeToken::BitAnd:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return BINARYOPER(BitAnd);
		}
		case HazeToken::BitOr:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return BINARYOPER(BitOr);
		}
		case HazeToken::BitXor:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return BINARYOPER(BitXor);
		}
		case HazeToken::Assign:
		{
			/*Share<CompilerValue> rightValue;
			if (dynamic_cast<ASTNew*>(m_RightAST.get()))
			{
				rightValue = dynamic_cast<ASTNew*>(m_RightAST.get())->CodeGen(m_LeftAST.get());
			}
			else
			{
				rightValue = right;
			}*/
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateMov(left, right);
		}
		case HazeToken::AddAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateAdd(left, left, right);
		}
		case HazeToken::SubAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateSub(left, left, right);
		}
		case HazeToken::MulAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateMul(left, left, right);
		}
		case HazeToken::DivAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateDiv(left, left, right);
		}
		case HazeToken::ModAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateMod(left, left, right);
		}
		case HazeToken::BitAndAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateBitAnd(left, left, right);
		}
		case HazeToken::BitOrAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateBitOr(left, left, right);
		}
		case HazeToken::BitXorAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateBitXor(left, left, right);
		}
		case HazeToken::ShlAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->CreateShl(left, left, right);
		}
		case HazeToken::ShrAssign:
		{
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();
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

			auto leftValue = m_LeftAST->CodeGen(nullptr);
			UPDATE_COMPILER_LINE_COUNT();
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

			auto rightValue = m_RightAST->CodeGen(nullptr);
			UPDATE_COMPILER_LINE_COUNT();
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

			auto leftValue = m_LeftAST->CodeGen(nullptr);
			UPDATE_COMPILER_LINE_COUNT();
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
			auto rightValue = m_RightAST->CodeGen(nullptr);
			UPDATE_COMPILER_LINE_COUNT();
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
			Share<CompilerValue> retValue = nullptr;
			ASSERT_GEN(left, m_LeftAST->CodeGen(inferValue));
			ASSERT_GEN(right, m_RightAST->CodeGen(left));
			UPDATE_COMPILER_LINE_COUNT();

			// 复杂类型
			if (!((left->IsNullPtr() && IsAdvanceType(right->GetBaseType())) || (right->IsNullPtr() && IsAdvanceType(left->GetBaseType()))) &&
				(IsAdvanceType(left->GetBaseType()) || IsAdvanceType(right->GetBaseType())))
			{
				if (!IsAdvanceType(left->GetBaseType()))
				{
					AST_ERR_W("<%s><%s>比较错误, <%s>不是复杂类型", m_LeftAST->GetName(), m_RightAST->GetName(), m_LeftAST->GetName());
				}
				else if (!IsAdvanceType(right->GetBaseType()))
				{
					AST_ERR_W("<%s><%s>比较错误, <%s>不是复杂类型", m_LeftAST->GetName(), m_RightAST->GetName(), m_RightAST->GetName());
				}
				else if (!(m_OperatorToken == HazeToken::Equal || m_OperatorToken == HazeToken::NotEqual))
				{
					AST_ERR_W("<%s><%s>比较错误, 只能比较等于或者不等于", m_LeftAST->GetName(), m_RightAST->GetName());
				}
				else if (left->GetTypeId() != right->GetTypeId())
				{
					AST_ERR_W("<%s><%s>比较错误, 不是相同类型", m_LeftAST->GetName(), m_RightAST->GetName());
				}
				else
				{
					if (m_OperatorToken == HazeToken::Equal)
					{
						retValue = m_Compiler->CreateAdvanceTypeFunctionCall(left->GetBaseType(), HAZE_ADVANCE_EQUAL_FUNCTION, { right }, left, false);
					}
					else if (m_OperatorToken == HazeToken::NotEqual)
					{
						retValue = m_Compiler->CreateAdvanceTypeFunctionCall(left->GetBaseType(), HAZE_ADVANCE_NOT_EQUAL_FUNCTION, { right }, left, false);
					}
				}
			}
			else
			{
				retValue = m_Compiler->CreateIntCmp(left, right);
			}
			
			if (m_LeftBlock || m_RightBlock)
			{
				if (m_ParentAst)
				{
					AST_ERR_W("<%s><%s>比较错误, 解析到父级操作", m_LeftAST->GetName(), m_RightAST->GetName());
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
	: ASTBase(compiler, location), m_ConditionAST(Move(conditionAST)), m_LeftAST(Move(leftAST)), m_RightAST(Move(rightAST)) {
}

Share<CompilerValue> ASTThreeExpression::CodeGen(Share<CompilerValue> inferValue)
{

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();

	auto parentBlock = m_Compiler->GetInsertBlock();

	auto defauleBlock = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
	auto blockLeft = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
	auto blockRight = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);

	auto conditionExp = dynamic_cast<ASTBinaryExpression*>(m_ConditionAST.get());
	if (conditionExp)
	{
		conditionExp->CodeGen(nullptr);
		m_Compiler->CreateCompareJmp(GetHazeCmpTypeByToken(conditionExp->m_OperatorToken), blockLeft, blockRight);
	}
	else
	{
		m_Compiler->CreateBoolCmp(m_ConditionAST->CodeGen(nullptr));
		m_Compiler->CreateCompareJmp(HazeCmpType::Equal, blockLeft, blockRight);
	}

	m_Compiler->SetInsertBlock(blockLeft);
	auto value = m_LeftAST->CodeGen(inferValue);
	auto retValue = m_Compiler->GetTempRegister(value);

	UPDATE_COMPILER_LINE_COUNT();
	m_Compiler->CreateMov(retValue, value);
	m_Compiler->CreateJmpToBlock(defauleBlock);

	m_Compiler->SetInsertBlock(blockRight);
	value = m_RightAST->CodeGen(inferValue);
	
	UPDATE_COMPILER_LINE_COUNT();
	m_Compiler->CreateMov(retValue, value);
	m_Compiler->CreateJmpToBlock(defauleBlock);

	m_Compiler->SetInsertBlock(defauleBlock);

	return retValue;
}

ASTImportModule::ASTImportModule(Compiler* compiler, const SourceLocation& location, STDString&& modulepath)
	: ASTBase(compiler, location), m_ModulePath(Move(modulepath))
{
}

Share<CompilerValue> ASTImportModule::CodeGen(Share<CompilerValue> inferValue)
{
	auto m = m_Compiler->ParseModuleByImportPath(m_ModulePath);
	if (m)
	{
		m_Compiler->AddImportModuleToCurrModule(m);
	}
	else
	{
		AST_ERR_W("引<%s>模块错误", m_ModulePath.c_str());
		m_Compiler->MarkCompilerError();
	}

	return nullptr;
}

ASTBreakExpression::ASTBreakExpression(Compiler* compiler, const SourceLocation& location)
	: ASTBase(compiler, location) {
}

Share<CompilerValue> ASTBreakExpression::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();
	m_Compiler->CreateJmpToBlock(m_Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopEnd()->GetShared());
	return nullptr;
}

ASTContinueExpression::ASTContinueExpression(Compiler* compiler, const SourceLocation& location)
	: ASTBase(compiler, location) {
}

Share<CompilerValue> ASTContinueExpression::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();
	m_Compiler->CreateJmpToBlock(m_Compiler->GetInsertBlock()->FindLoopBlock()->GetLoopStep()->GetShared());
	return nullptr;
}

ASTIfExpression::ASTIfExpression(Compiler* compiler, const SourceLocation& location, const SourceLocation& finishLocation, Unique<ASTBase>& condition,
	Unique<ASTBase>& ifExpression, Unique<ASTBase>& elseExpression)
	: ASTBase(compiler, location), m_FinishLocation(finishLocation), m_Condition(Move(condition)), m_IfExpression(Move(ifExpression)),
	m_ElseExpression(Move(elseExpression))
{
}

Share<CompilerValue> ASTIfExpression::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();

	auto parentBlock = m_Compiler->GetInsertBlock();
	auto ifThenBlock = CompilerBlock::CreateBaseBlock(function->GenIfThenBlockName(), function, parentBlock);
	auto elseBlock = m_ElseExpression ? CompilerBlock::CreateBaseBlock(function->GenElseBlockName(), function, parentBlock) : nullptr;
	auto nextBlock = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);

	auto ConditionExp = dynamic_cast<ASTBinaryExpression*>(m_Condition.get());

	if (ConditionExp && GetHazeCmpTypeByToken(ConditionExp->m_OperatorToken) != HazeCmpType::None)
	{
		ConditionExp->SetLeftAndRightBlock(ifThenBlock.get(), m_ElseExpression ? elseBlock.get() : nextBlock.get(), nullptr);
		m_Condition->CodeGen(nullptr);
	}
	else
	{
		m_Compiler->CreateBoolCmp(m_Condition->CodeGen(nullptr));
		m_Compiler->CreateCompareJmp(HazeCmpType::Equal, ifThenBlock, m_ElseExpression ? elseBlock : nextBlock);
	}

	m_Compiler->SetInsertBlock(ifThenBlock);
	m_IfExpression->CodeGen(nullptr);
	
	UPDATE_COMPILER_LINE_COUNT1(m_FinishLocation.Line);
	m_Compiler->CreateJmpToBlock(nextBlock);

	if (m_ElseExpression)
	{
		m_Compiler->SetInsertBlock(elseBlock);
		m_ElseExpression->CodeGen(nullptr);
		
		UPDATE_COMPILER_LINE_COUNT1(m_FinishLocation.Line);
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
	:ASTBase(compiler, location), m_Condition(Move(condition)), m_MultiExpression(Move(multiExpression)) {
}

Share<CompilerValue> ASTWhileExpression::CodeGen(Share<CompilerValue> inferValue)
{

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
		m_Condition->CodeGen(nullptr);
	}
	else
	{
		UPDATE_COMPILER_LINE_COUNT();
		m_Compiler->CreateBoolCmp(m_Condition->CodeGen(nullptr));
		m_Compiler->CreateCompareJmp(HazeCmpType::Equal, whileBlock, nextBlock);
	}

	m_Compiler->SetInsertBlock(whileBlock);
	m_MultiExpression->CodeGen(nullptr);
	
	UPDATE_COMPILER_LINE_COUNT();
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

Share<CompilerValue> ASTForExpression::CodeGen(Share<CompilerValue> inferValue)
{
	UPDATE_COMPILER_LINE_COUNT();

	auto function = m_Compiler->GetCurrModule()->GetCurrFunction();
	auto parentBlock = m_Compiler->GetInsertBlock();

	auto forConditionBlock = CompilerBlock::CreateBaseBlock(function->GenForConditionBlockName(), function, parentBlock);
	auto loopBlock = CompilerBlock::CreateBaseBlock(function->GenLoopBlockName(), function, parentBlock);
	auto forStepBlock = CompilerBlock::CreateBaseBlock(function->GenForStepBlockName(), function, parentBlock);
	auto endLoopBlock = CompilerBlock::CreateBaseBlock(function->GenDafaultBlockName(), function, parentBlock);
	loopBlock->SetLoopEnd(endLoopBlock.get());
	loopBlock->SetLoopStep(forStepBlock.get());

	if (!m_InitExpression->CodeGen(nullptr))
	{
		AST_ERR_W("循环语句初始化失败");
		return nullptr;
	}

	m_Compiler->CreateJmpToBlock(forConditionBlock);
	m_Compiler->SetInsertBlock(forConditionBlock);
	auto conditionExp = dynamic_cast<ASTBinaryExpression*>(m_ConditionExpression.get());
	if (conditionExp)
	{
		conditionExp->SetLeftAndRightBlock(loopBlock.get(), endLoopBlock.get(), nullptr);
		m_ConditionExpression->CodeGen(nullptr);
	}
	else
	{
		UPDATE_COMPILER_LINE_COUNT();
		m_Compiler->CreateBoolCmp(m_ConditionExpression->CodeGen(nullptr));
		m_Compiler->CreateCompareJmp(HazeCmpType::Equal, loopBlock, endLoopBlock);
	}

	m_Compiler->SetInsertBlock(loopBlock);
	m_MultiExpression->CodeGen(nullptr);

	UPDATE_COMPILER_LINE_COUNT();
	m_Compiler->CreateJmpToBlock(forStepBlock);

	m_Compiler->SetInsertBlock(forStepBlock);
	m_StepExpression->CodeGen(nullptr);


	UPDATE_COMPILER_LINE_COUNT();
	m_Compiler->CreateJmpToBlock(forConditionBlock);

	m_Compiler->SetInsertBlock(endLoopBlock);

	return nullptr;
}

ASTCast::ASTCast(Compiler* compiler, const SourceLocation& location, HazeVariableType& type, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression))
{
	m_DefineVariable.Type = type;
}

Share<CompilerValue> ASTCast::CodeGen(Share<CompilerValue> inferValue)
{
	auto value = m_Expression->CodeGen(nullptr);
	if (value)
	{
		UPDATE_COMPILER_LINE_COUNT();
		return m_Compiler->CreateCast(m_DefineVariable.Type, value);
	}
	else
	{
		AST_ERR_W("强转<%s>错误", m_Expression->GetName());
	}

	return nullptr;
}

ASTSizeOf::ASTSizeOf(Compiler* compiler, const SourceLocation& location, const HazeVariableType& type, Unique<ASTBase>& expression)
	: ASTBase(compiler, location), m_Expression(Move(expression))
{
	m_DefineVariable.Type = type;
}

Share<CompilerValue> ASTSizeOf::CodeGen(Share<CompilerValue> inferValue)
{
	if (m_Expression)
	{
		auto value = m_Expression->CodeGen(nullptr);
		if (value)
		{
			UPDATE_COMPILER_LINE_COUNT();
			return m_Compiler->GetConstantValueUint64(value->GetSize());
		}
		else
		{
			AST_ERR_W("获得<%s>大小错误", m_Expression->GetName());
		}
	}
	else
	{
		UPDATE_COMPILER_LINE_COUNT();
		return m_Compiler->GetConstantValueUint64(m_DefineVariable.Type.GetTypeSize());
	}

	return nullptr;
}