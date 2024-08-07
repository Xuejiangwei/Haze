#pragma once
#include "HazeHeader.h"
#include "HazeDebugInfo.h"

class HazeCompiler;
class HazeCompilerValue;
class HazeBaseBlock;

//Base
class ASTBase
{
	friend class ASTClass;
	friend class ASTVariableDefine_Function;
public:
	ASTBase(HazeCompiler* compiler, const SourceLocation& location);

	ASTBase(HazeCompiler* compiler, const SourceLocation& location, const HazeDefineVariable& defVar);
	
	virtual ~ASTBase() {}

	virtual Share<HazeCompilerValue> CodeGen() { return  nullptr; }

	virtual const HChar* GetName() { return H_TEXT(""); }

	virtual bool IsBlock() const { return false; }

	const HazeDefineVariable& GetDefine() const { return m_DefineVariable; }

protected:
	HazeCompiler* m_Compiler;
	HazeValue m_Value;
	HazeDefineVariable m_DefineVariable;

	SourceLocation m_Location;
};

//布尔
class ASTBool : public ASTBase
{
public:
	ASTBool(HazeCompiler* compiler, const SourceLocation& location, const HazeValue& value);

	virtual ~ASTBool() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;
};

//数字
class ASTNumber : public ASTBase
{
public:
	ASTNumber(HazeCompiler* compiler, const SourceLocation& location, HazeValueType type, const HazeValue& value);
	
	virtual ~ASTNumber() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;
};

//字符串
class ASTStringText : public ASTBase
{
public:
	ASTStringText(HazeCompiler* compiler, const SourceLocation& location, HString& text);
	
	virtual ~ASTStringText() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	HString m_Text;
};

//变量
class ASTIdentifier : public ASTBase
{
public:
	ASTIdentifier(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, HString& name,
		V_Array<Unique<ASTBase>>& arrayIndexExpression, HString nameSpace = HAZE_TEXT(""));
	
	virtual ~ASTIdentifier() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;

	virtual const HChar* GetName() { return m_DefineVariable.Name.c_str(); }

private:
	HString m_NameSpace;
	HazeSectionSignal m_SectionSignal;
	V_Array<Unique<ASTBase>> m_ArrayIndexExpression;
};

//函数调用
class ASTFunctionCall : public ASTBase
{
public:
	ASTFunctionCall(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, HString& name
		, V_Array<Unique<ASTBase>>& functionParam, Unique<ASTBase> classObj = nullptr);
	
	virtual ~ASTFunctionCall() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;

	virtual const HChar* GetName() { return m_Name.c_str(); }

private:
	HazeSectionSignal m_SectionSignal;
	HString m_Name;
	Unique<ASTBase> m_ClassObj;
	V_Array<Unique<ASTBase>> m_FunctionParam;

};

class ASTClassAttr : public ASTBase
{
public:
	ASTClassAttr(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		HString& classObjName, HString& attrName, bool isFunction, V_Array<Unique<ASTBase>>* functionParam = nullptr);

	virtual ~ASTClassAttr() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	bool m_IsAdvanceType;
	bool m_IsFunction;
	HazeSectionSignal m_SectionSignal;
	HString m_AttrName;
	V_Array<Unique<ASTBase>> m_Params;
};

//变量定义 基本类型 类对象
class ASTVariableDefine : public ASTBase
{
public:
	ASTVariableDefine(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase> expression);

	virtual ~ASTVariableDefine() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;

	virtual const HChar* GetName() { return m_DefineVariable.Name.c_str(); }

	Share<HazeCompilerValue> GenExpressionValue();

protected:
	HazeSectionSignal m_SectionSignal;
	Unique<ASTBase> m_Expression;
};

class ASTVariableDefine_MultiVariable : public ASTBase
{
public:
	ASTVariableDefine_MultiVariable(HazeCompiler* compiler, const SourceLocation& location);

	virtual ~ASTVariableDefine_MultiVariable() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;
};

class ASTVariableDefine_Class : public ASTVariableDefine
{
public:
	ASTVariableDefine_Class(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase> expression, TemplateDefineTypes& templateTypes, V_Array<Unique<ASTBase>> params = {});

	virtual ~ASTVariableDefine_Class() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;

protected:
	TemplateDefineTypes m_TemplateTypes;

	union
	{
		V_Array<Unique<ASTBase>> m_Params;
		V_Array<Unique<ASTBase>> m_ArraySize;
	}; 
};

//变量定义 数组
class ASTVariableDefine_Array : public ASTVariableDefine_Class
{
public:
	ASTVariableDefine_Array(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase> expression, TemplateDefineTypes& templateTypes,
		V_Array<Unique<ASTBase>>& arraySize);

	virtual ~ASTVariableDefine_Array() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;
};

//变量定义 函数
class ASTVariableDefine_Function : public ASTVariableDefine
{
public:
	ASTVariableDefine_Function(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase> expression, TemplateDefineTypes& templateTypes);
	
	virtual ~ASTVariableDefine_Function() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	TemplateDefineTypes m_TemplateTypes;
};

//返回
class ASTReturn : public ASTBase
{
public:
	ASTReturn(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression);
	
	virtual ~ASTReturn() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	Unique<ASTBase> m_Expression;
};

//New
class ASTNew : public ASTBase
{
public:
	ASTNew(HazeCompiler* compiler, const SourceLocation& location, const HazeDefineVariable& defineVar, V_Array<Unique<ASTBase>> countarrayExpression = {},
		V_Array<Unique<ASTBase>> constructorParam = {});
	
	virtual ~ASTNew() override {}

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	V_Array<Unique<ASTBase>> m_CountArrayExpression;
	V_Array<Unique<ASTBase>> m_ConstructorParam;
};

//获得地址
class ASTGetAddress : public ASTBase
{
public:
	ASTGetAddress(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression);

	virtual ~ASTGetAddress() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	Unique<ASTBase> m_Expression;
};

//Parse pointer value
class ASTPointerValue : public ASTBase
{
public:
	ASTPointerValue(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, 
		int level, Unique<ASTBase> assignExpression = nullptr);
	
	virtual ~ASTPointerValue() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	Unique<ASTBase> m_Expression;
	Unique<ASTBase> m_AssignExpression;
	int m_Level;
};

//Parse pointer value
class ASTNeg : public ASTBase
{
public:
	ASTNeg(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression,
		bool isNumberNeg);

	virtual ~ASTNeg() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	Unique<ASTBase> m_Expression;
	bool m_IsNumberNeg;
};

//Null pointer
class ASTNullPtr : public ASTBase
{
public:
	ASTNullPtr(HazeCompiler* compiler, const SourceLocation& location);

	virtual ~ASTNullPtr() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

	void SetDefineType(const HazeDefineType& type);
};

//非 表达式
class ASTNot : public ASTBase
{
public:
	ASTNot(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression);
	
	virtual ~ASTNot() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	Unique<ASTBase> m_Expression;
};

//Inc
class ASTInc : public ASTBase
{
public:
	ASTInc(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression,
		bool isPreInc);

	virtual ~ASTInc() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	Unique<ASTBase> m_Expression;
	bool m_IsPreInc;
};

//Dec
class ASTDec : public ASTBase
{
public:
	ASTDec(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression,
		bool isPreDec);
	
	virtual ~ASTDec() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	Unique<ASTBase> m_Expression;
	bool m_IsPreDec;
};

//Operator
class ASTOperetorAssign : public ASTBase
{
public:
	ASTOperetorAssign(HazeCompiler* compiler, const SourceLocation& location, HazeToken token, 
		Unique<ASTBase>& expression);

	virtual ~ASTOperetorAssign() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	HazeToken m_Token;
	Unique<ASTBase> m_Expression;
};

//二元表达式
class ASTBinaryExpression : public ASTBase
{
	friend class ASTIfExpression;
	friend class ASTWhileExpression;
	friend class ASTForExpression;
	friend class ASTThreeExpression;
public:
	ASTBinaryExpression(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		HazeToken operatorToken, Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST);

	virtual ~ASTBinaryExpression() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

	void SetLeftAndRightBlock(HazeBaseBlock* leftJmpBlock, HazeBaseBlock* rightJmpBlock);

private:
	HazeSectionSignal m_SectionSignal;
	HazeToken m_OperatorToken;

	Unique<ASTBase> m_LeftAST;
	Unique<ASTBase> m_RightAST;

	HazeBaseBlock* m_LeftBlock;
	HazeBaseBlock* m_RightBlock;
	HazeBaseBlock* m_DefaultBlock;
};

//三目表达式
class ASTThreeExpression : public ASTBase
{
	friend class ASTIfExpression;
	friend class ASTWhileExpression;
	friend class ASTForExpression;
public:
	ASTThreeExpression(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& conditionAST, 
		Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST);
	
	virtual ~ASTThreeExpression() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const override { return true; }

private:
	Unique<ASTBase> m_ConditionAST;
	Unique<ASTBase> m_LeftAST;
	Unique<ASTBase> m_RightAST;
};

//多行表达式
class ASTMultiExpression : public ASTBase
{
public:
	ASTMultiExpression(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		V_Array<Unique<ASTBase>>& expressions);

	virtual ~ASTMultiExpression() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	HazeSectionSignal m_SectionSignal;
	V_Array<Unique<ASTBase>> m_Expressions;
};

//引用库
class ASTImportModule : public ASTBase
{
public:
	ASTImportModule(HazeCompiler* compiler, const SourceLocation& location, const HString& modulepath);
	
	virtual ~ASTImportModule() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	HString m_ModulePath;
};

//Break
class ASTBreakExpression : public ASTBase
{
public:
	ASTBreakExpression(HazeCompiler* compiler, const SourceLocation& location);

	virtual ~ASTBreakExpression() override;

	virtual Share<HazeCompilerValue> CodeGen() override;
};

//Continue
class ASTContinueExpression : public ASTBase
{
public:
	ASTContinueExpression(HazeCompiler* compiler, const SourceLocation& location);

	virtual ~ASTContinueExpression() override;

	virtual Share<HazeCompilerValue> CodeGen() override;
};

//若
class ASTIfExpression : public ASTBase
{
public:
	ASTIfExpression(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& condition,
		Unique<ASTBase>& ifExpression, Unique<ASTBase>& elseExpression);
	
	virtual ~ASTIfExpression() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

	bool HasElseExpression() const;

private:
	Unique<ASTBase> m_Condition;
	Unique<ASTBase> m_IfExpression;
	Unique<ASTBase> m_ElseExpression;
};

//当
class ASTWhileExpression : public ASTBase
{
public:
	ASTWhileExpression(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& condition,
		Unique<ASTBase>& multiExpression);
	
	virtual ~ASTWhileExpression() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

private:
	Unique<ASTBase> m_Condition;
	Unique<ASTBase> m_MultiExpression;
};

//循环
class ASTForExpression : public ASTBase
{
public:
	ASTForExpression(HazeCompiler* compiler, const SourceLocation& location, Unique<ASTBase>& initExpression,
		Unique<ASTBase>& conditionExpression, Unique<ASTBase>& stepExpression, 
		Unique<ASTBase>& multiExpression);

	virtual	~ASTForExpression()	override;

	virtual Share<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

private:
	Unique<ASTBase> m_InitExpression;
	Unique<ASTBase> m_ConditionExpression;
	Unique<ASTBase> m_StepExpression;
	Unique<ASTBase> m_MultiExpression;
};

//初始化列表
class ASTInitializeList : public ASTBase
{
public:
	ASTInitializeList(HazeCompiler* compiler, const SourceLocation& location, 
		V_Array<Unique<ASTBase>>& initializeListExpression);

	virtual	~ASTInitializeList() override;

	virtual Share<HazeCompilerValue> CodeGen() override;

private:
	V_Array<Unique<ASTBase>> m_InitializeListExpression;
};

//强转
class ASTCast : public ASTBase
{
public:
	ASTCast(HazeCompiler* compiler, const SourceLocation& location, HazeDefineType& type, Unique<ASTBase>& expression);

	virtual ~ASTCast() override;

	virtual Share<HazeCompilerValue> CodeGen();

private:
	Unique<ASTBase> m_Expression;
};

//获得字节大小
class ASTSizeOf : public ASTBase
{
public:
	ASTSizeOf(HazeCompiler* compiler, const SourceLocation& location, const HazeDefineType& type, Unique<ASTBase>& expression);

	virtual ~ASTSizeOf() override;

	virtual Share<HazeCompilerValue> CodeGen();

private:
	Unique<ASTBase> m_Expression;
};