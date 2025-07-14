#pragma once
#include "HazeHeader.h"
#include "HazeDebugInfo.h"

class Compiler;
class CompilerValue;
class CompilerBlock;

//Base abstract syntax tree
class ASTBase
{
	friend class ASTClass;
	friend class ASTVariableDefine_Function;
public:
	ASTBase(Compiler* compiler, const SourceLocation& location);
	ASTBase(Compiler* compiler, const SourceLocation& location, const HazeDefineVariable& defVar);
	virtual ~ASTBase() {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) { return nullptr; }

	virtual const x_HChar* GetName() { return H_TEXT(""); }

	virtual bool IsBlock() const { return false; }

	const HazeDefineVariable& GetDefine() const { return m_DefineVariable; }

protected:
	Compiler* m_Compiler;
	HazeValue m_Value;
	HazeDefineVariable m_DefineVariable;
	SourceLocation m_Location;
};

//布尔
class ASTBool : public ASTBase
{
public:
	ASTBool(Compiler* compiler, const SourceLocation& location, const HazeValue& value);
	virtual ~ASTBool() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
};

//数字
class ASTNumber : public ASTBase
{
public:
	ASTNumber(Compiler* compiler, const SourceLocation& location, HazeValueType type, const HazeValue& value);
	virtual ~ASTNumber() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
};

//字符串
class ASTStringText : public ASTBase
{
public:
	ASTStringText(Compiler* compiler, const SourceLocation& location, HString& text);
	virtual ~ASTStringText() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	HString m_Text;
};

//变量
class ASTIdentifier : public ASTBase
{
public:
	ASTIdentifier(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, HString& name,
		Unique<ASTBase> arrayIndexExpression, Unique<ASTBase> preAst, HString nameSpace);
	virtual ~ASTIdentifier() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
	virtual const x_HChar* GetName() { return m_DefineVariable.Name.c_str(); }

	const HString& GetNameSpace() const { return m_NameSpace; }

private:
	Share<CompilerValue> GetNameValue();

private:
	HString m_NameSpace;
	HazeSectionSignal m_SectionSignal;
	Unique<ASTBase> m_PreAst;
	Unique<ASTBase> m_ArrayIndexExpression;
};

//函数调用
class ASTFunctionCall : public ASTBase
{
public:
	ASTFunctionCall(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, HString& name
		, V_Array<Unique<ASTBase>>& functionParam, Unique<ASTBase> classObj, HString nameSpace);
	virtual ~ASTFunctionCall() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
	virtual const x_HChar* GetName() { return m_Name.c_str(); }

private:
	HString m_NameSpace;
	HazeSectionSignal m_SectionSignal;
	HString m_Name;
	Unique<ASTBase> m_ClassObj;
	V_Array<Unique<ASTBase>> m_FunctionParam;
};

//class ASTClassAttr : public ASTBase
//{
//public:
//	ASTClassAttr(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, HString& classObjName, 
//		Unique<ASTBase>& preAst, HString& attrName, bool isFunction, V_Array<Unique<ASTBase>>* functionParam = nullptr);
//	virtual ~ASTClassAttr() override {}
//
//	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
//
//private:
//	bool m_IsFunction;
//	HazeSectionSignal m_SectionSignal;
//	HString m_AttrName;
//	Unique<ASTBase> m_PreAst;
//	V_Array<Unique<ASTBase>> m_Params;
//};

//变量定义 基本类型 类对象
class ASTVariableDefine : public ASTBase
{
public:
	ASTVariableDefine(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase> expression);
	virtual ~ASTVariableDefine() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
	virtual const x_HChar* GetName() { return m_DefineVariable.Name.c_str(); }

	Share<CompilerValue> GenExpressionValue(Share<CompilerValue> value);

	HazeSectionSignal GetSectionSingal() const { return m_SectionSignal; }

protected:
	Share<CompilerValue> TryAssign(Share<CompilerValue> var, const x_HChar* errorPrefix);

	HazeSectionSignal m_SectionSignal;
	Unique<ASTBase> m_Expression;
};

class ASTVariableDefine_MultiVariable : public ASTBase
{
public:
	ASTVariableDefine_MultiVariable(Compiler* compiler, const SourceLocation& location);
	virtual ~ASTVariableDefine_MultiVariable() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
};

class ASTVariableDefine_Class : public ASTVariableDefine
{
public:
	ASTVariableDefine_Class(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase> expression, V_Array<Unique<ASTBase>> params = {});
	virtual ~ASTVariableDefine_Class() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

protected:
	V_Array<Unique<ASTBase>> m_Params;
};

//变量定义 数组
class ASTVariableDefine_Array : public ASTVariableDefine
{
public:
	ASTVariableDefine_Array(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase> expression, x_uint64 dimension);
	virtual ~ASTVariableDefine_Array() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

protected:
	TemplateDefineTypes m_TemplateTypes;
	x_uint64 m_ArrayDimension;
};

//变量定义 函数
class ASTVariableDefine_Function : public ASTVariableDefine
{
public:
	ASTVariableDefine_Function(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase> expression, x_uint32 templateTypeId);
	virtual ~ASTVariableDefine_Function() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

protected:
	x_uint32 m_TemplateTypeId;
};

//变量定义 基本类型对象
class ASTVariableDefine_ObjectBase : public ASTVariableDefine
{
public:
	ASTVariableDefine_ObjectBase(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase>& expression);
	virtual ~ASTVariableDefine_ObjectBase() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
};

//变量定义 哈希对象
class ASTVariableDefine_Hash : public ASTVariableDefine
{
public:
	ASTVariableDefine_Hash(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase>& expression, x_uint32 templateTypeId);
	virtual ~ASTVariableDefine_Hash() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	x_uint32 m_TemplateTypeId;
	Unique<ASTBase> m_Expression;
};

//变量定义 闭包对象
class ASTVariableDefine_Closure : public ASTVariableDefine_Function
{
public:
	ASTVariableDefine_Closure(Compiler* compiler, const SourceLocation& location, const SourceLocation& startLocation, const SourceLocation& endLocation, HazeSectionSignal section,
		const HazeDefineVariable& defineVar, Unique<ASTBase>& expression, x_uint32 templateTypeId, V_Array<Unique<ASTBase>>& params);
	virtual ~ASTVariableDefine_Closure() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	V_Array<Unique<ASTBase>> m_Params;
	SourceLocation m_StartLocation;
	SourceLocation m_EndLocation;
};

//class ASTClosure : public ASTBase
//{
//public:
//	ASTClosure(Compiler* compiler, const SourceLocation& location, const SourceLocation& startLocation, const SourceLocation& endLocation, HazeSectionSignal section,
//		Unique<ASTBase>& expression, V_Array<Unique<ASTBase>>& params);
//
//	virtual ~ASTClosure() override;
//
//	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
//
//private:
//	Unique<ASTBase> m_Expression;
//	V_Array<Unique<ASTBase>> m_Params;
//	SourceLocation m_StartLocation;
//	SourceLocation m_EndLocation;
//};

//返回
class ASTReturn : public ASTBase
{
public:
	ASTReturn(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression);
	virtual ~ASTReturn() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	Unique<ASTBase> m_Expression;
};

//生成
class ASTNew : public ASTBase
{
public:
	ASTNew(Compiler* compiler, const SourceLocation& location, const HazeDefineVariable& defineVar, /*TemplateDefineTypes& templateTypes,*/ V_Array<Unique<ASTBase>> countArrayExpression = {},
		V_Array<Unique<ASTBase>> constructorParam = {});
	virtual ~ASTNew() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	V_Array<Unique<ASTBase>> m_CountArrayExpression;
	V_Array<Unique<ASTBase>> m_ConstructorParam;
	//TemplateDefineTypes m_TemplateTypes;
};

//获得地址
class ASTGetAddress : public ASTBase
{
public:
	ASTGetAddress(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression);
	virtual ~ASTGetAddress() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
private:
	Unique<ASTBase> m_Expression;
};

class ASTNeg : public ASTBase
{
public:
	ASTNeg(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression,
		bool isNumberNeg);
	virtual ~ASTNeg() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	Unique<ASTBase> m_Expression;
	bool m_IsNumberNeg;
};

//Null pointer
class ASTNullPtr : public ASTBase
{
public:
	ASTNullPtr(Compiler* compiler, const SourceLocation& location);
	virtual ~ASTNullPtr() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

	void SetDefineType(const HazeVariableType& type);
};

//非 表达式
class ASTNot : public ASTBase
{
public:
	ASTNot(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression);
	virtual ~ASTNot() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	Unique<ASTBase> m_Expression;
};

//Inc
class ASTInc : public ASTBase
{
public:
	ASTInc(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreInc);
	virtual ~ASTInc() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	Unique<ASTBase> m_Expression;
	bool m_IsPreInc;
};

//Dec
class ASTDec : public ASTBase
{
public:
	ASTDec(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& expression, bool isPreDec);
	virtual ~ASTDec() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	Unique<ASTBase> m_Expression;
	bool m_IsPreDec;
};

//二元表达式
class ASTBinaryExpression : public ASTBase
{
	friend class ASTIfExpression;
	friend class ASTWhileExpression;
	friend class ASTForExpression;
	friend class ASTThreeExpression;
public:
	ASTBinaryExpression(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		HazeToken operatorToken, Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST);
	virtual ~ASTBinaryExpression() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

	void SetLeftAndRightBlock(CompilerBlock* leftJmpBlock, CompilerBlock* rightJmpBlock, ASTBinaryExpression* parentAst);

	CompilerBlock* TryShortCircuit(ASTBinaryExpression* ast);

	void SetAssignToAst(ASTBase* assignToAst) { m_AssignToAst = assignToAst; }

private:
	void SetChildBlock();

private:
	HazeSectionSignal m_SectionSignal;
	HazeToken m_OperatorToken;
	
	Unique<ASTBase> m_LeftAST;
	Unique<ASTBase> m_RightAST;

	ASTBase* m_AssignToAst;

	CompilerBlock* m_LeftBlock;
	CompilerBlock* m_RightBlock;
	ASTBinaryExpression* m_ParentAst;
	Share<CompilerBlock> m_ShortCircuitBlock;
};

//三目表达式
class ASTThreeExpression : public ASTBase
{
	friend class ASTIfExpression;
	friend class ASTWhileExpression;
	friend class ASTForExpression;
public:
	ASTThreeExpression(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& conditionAST, 
		Unique<ASTBase>& leftAST, Unique<ASTBase>& rightAST);
	virtual ~ASTThreeExpression() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

	virtual bool IsBlock() const override { return true; }

private:
	Unique<ASTBase> m_ConditionAST;
	Unique<ASTBase> m_LeftAST;
	Unique<ASTBase> m_RightAST;
};

class ASTDataSection : public ASTBase
{
public:
	ASTDataSection(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, V_Array<Unique<ASTBase>>& expressions);
	virtual ~ASTDataSection() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	HazeSectionSignal m_SectionSignal;
	V_Array<Unique<ASTBase>> m_Expressions;
};

//多行表达式
class ASTMultiExpression : public ASTBase
{
public:
	ASTMultiExpression(Compiler* compiler, const SourceLocation& location, HazeSectionSignal section, V_Array<Unique<ASTBase>>& expressions);
	virtual ~ASTMultiExpression() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	HazeSectionSignal m_SectionSignal;
	V_Array<Unique<ASTBase>> m_Expressions;
};

//引用库
class ASTImportModule : public ASTBase
{
public:
	ASTImportModule(Compiler* compiler, const SourceLocation& location, const HString& modulepath);
	virtual ~ASTImportModule() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	HString m_ModulePath;
};

//Break
class ASTBreakExpression : public ASTBase
{
public:
	ASTBreakExpression(Compiler* compiler, const SourceLocation& location);
	virtual ~ASTBreakExpression() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
};

//Continue
class ASTContinueExpression : public ASTBase
{
public:
	ASTContinueExpression(Compiler* compiler, const SourceLocation& location);
	virtual ~ASTContinueExpression() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;
};

//若
class ASTIfExpression : public ASTBase
{
public:
	ASTIfExpression(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& condition,
		Unique<ASTBase>& ifExpression, Unique<ASTBase>& elseExpression);
	virtual ~ASTIfExpression() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

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
	ASTWhileExpression(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& condition,
		Unique<ASTBase>& multiExpression);
	virtual ~ASTWhileExpression() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

	virtual bool IsBlock() const { return true; }

private:
	Unique<ASTBase> m_Condition;
	Unique<ASTBase> m_MultiExpression;
};

//循环
class ASTForExpression : public ASTBase
{
public:
	ASTForExpression(Compiler* compiler, const SourceLocation& location, Unique<ASTBase>& initExpression,
		Unique<ASTBase>& conditionExpression, Unique<ASTBase>& stepExpression, Unique<ASTBase>& multiExpression);
	virtual	~ASTForExpression()	override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

	virtual bool IsBlock() const { return true; }

private:
	Unique<ASTBase> m_InitExpression;
	Unique<ASTBase> m_ConditionExpression;
	Unique<ASTBase> m_StepExpression;
	Unique<ASTBase> m_MultiExpression;
};

//强转
class ASTCast : public ASTBase
{
public:
	ASTCast(Compiler* compiler, const SourceLocation& location, HazeVariableType& type, Unique<ASTBase>& expression);
	virtual ~ASTCast() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	Unique<ASTBase> m_Expression;
};

//获得字节大小
class ASTSizeOf : public ASTBase
{
public:
	ASTSizeOf(Compiler* compiler, const SourceLocation& location, const HazeVariableType& type, Unique<ASTBase>& expression);
	virtual ~ASTSizeOf() override {}

	virtual Share<CompilerValue> CodeGen(Share<CompilerValue> inferValue) override;

private:
	Unique<ASTBase> m_Expression;
};