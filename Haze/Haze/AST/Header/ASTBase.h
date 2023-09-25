#pragma once

#include <vector>
#include <memory>

#include "Haze.h"
#include "HazeDebugInfo.h"

class HazeCompiler;
class HazeCompilerValue;
class HazeBaseBlock;

//Base
class ASTBase
{
	friend class ASTClass;

public:
	ASTBase(HazeCompiler* compiler, const SourceLocation& location);

	ASTBase(HazeCompiler* compiler, const SourceLocation& location, const HazeDefineVariable& defVar);
	
	virtual ~ASTBase();

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() { return  nullptr; }

	virtual const HAZE_CHAR* GetName() { return HAZE_TEXT(""); }

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

	virtual ~ASTBool() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//数字
class ASTNumber : public ASTBase
{
public:
	ASTNumber(HazeCompiler* compiler, const SourceLocation& location, HazeValueType type, const HazeValue& value);
	
	virtual ~ASTNumber() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//字符串
class ASTStringText : public ASTBase
{
public:
	ASTStringText(HazeCompiler* compiler, const SourceLocation& location, HAZE_STRING& text);
	
	virtual ~ASTStringText() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HAZE_STRING m_Text;
};

//变量
class ASTIdentifier : public ASTBase
{
public:
	ASTIdentifier(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, HAZE_STRING& name
		, std::vector<std::unique_ptr<ASTBase>>& arrayIndexExpression);
	
	virtual ~ASTIdentifier() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual const HAZE_CHAR* GetName() { return m_DefineVariable.m_Name.c_str(); }

private:
	HazeSectionSignal m_SectionSignal;
	std::vector<std::unique_ptr<ASTBase>> m_ArrayIndexExpression;
};

//函数调用
class ASTFunctionCall : public ASTBase
{
public:
	ASTFunctionCall(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, HAZE_STRING& name
		, std::vector<std::unique_ptr<ASTBase>>& functionParam);
	
	virtual ~ASTFunctionCall() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual const HAZE_CHAR* GetName() { return m_Name.c_str(); }

private:
	HazeSectionSignal m_SectionSignal;
	HAZE_STRING m_Name;

	std::vector<std::unique_ptr<ASTBase>> m_FunctionParam;
};

//变量定义
class ASTVariableDefine : public ASTBase
{
public:
	ASTVariableDefine(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section, 
		const HazeDefineVariable& defineVar, std::unique_ptr<ASTBase> expression, 
		std::vector<std::unique_ptr<ASTBase>> arraySize = {}, int pointerLevel = 0, std::vector<HazeDefineType>* paramType = nullptr);

	virtual ~ASTVariableDefine() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual const HAZE_CHAR* GetName() { return m_DefineVariable.m_Name.c_str(); }

private:
	HazeSectionSignal m_SectionSignal;
	std::unique_ptr<ASTBase> m_Expression;
	std::vector<std::unique_ptr<ASTBase>> m_ArraySize;
	int m_PointerLevel;

	std::vector<HazeDefineType> m_Vector_PointerFunctionParamType;
};

//返回
class ASTReturn : public ASTBase
{
public:
	ASTReturn(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression);
	
	virtual ~ASTReturn() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> m_Expression;
};

//New
class ASTNew : public ASTBase
{
public:
	ASTNew(HazeCompiler* compiler, const SourceLocation& location, const HazeDefineVariable& defineVar);
	
	virtual ~ASTNew() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//获得地址
class ASTGetAddress : public ASTBase
{
public:
	ASTGetAddress(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression);

	virtual ~ASTGetAddress() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> m_Expression;
};

//Parse pointer value
class ASTPointerValue : public ASTBase
{
public:
	ASTPointerValue(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression, 
		int level, std::unique_ptr<ASTBase> assignExpression = nullptr);
	
	virtual ~ASTPointerValue() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> m_Expression;
	std::unique_ptr<ASTBase> m_AssignExpression;
	int m_Level;
};

//Parse pointer value
class ASTNeg : public ASTBase
{
public:
	ASTNeg(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression,
		bool isNumberNeg);

	virtual ~ASTNeg() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> m_Expression;
	bool m_IsNumberNeg;
};

//Null pointer
class ASTNullPtr : public ASTBase
{
public:
	ASTNullPtr(HazeCompiler* compiler, const SourceLocation& location);

	virtual ~ASTNullPtr() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	void SetDefineType(const HazeDefineType& type);
};

//非 表达式
class ASTNot : public ASTBase
{
public:
	ASTNot(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression);
	
	virtual ~ASTNot() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> m_Expression;
};

//Inc
class ASTInc : public ASTBase
{
public:
	ASTInc(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression,
		bool isPreInc);

	virtual ~ASTInc() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> m_Expression;
	bool m_IsPreInc;
};

//Dec
class ASTDec : public ASTBase
{
public:
	ASTDec(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& expression,
		bool isPreDec);
	
	virtual ~ASTDec() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> m_Expression;
	bool m_IsPreDec;
};

//Operator
class ASTOperetorAssign : public ASTBase
{
public:
	ASTOperetorAssign(HazeCompiler* compiler, const SourceLocation& location, HazeToken token, 
		std::unique_ptr<ASTBase>& expression);

	virtual ~ASTOperetorAssign() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HazeToken m_Token;
	std::unique_ptr<ASTBase> m_Expression;
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
		HazeToken operatorToken, std::unique_ptr<ASTBase>& leftAST, std::unique_ptr<ASTBase>& rightAST);

	virtual ~ASTBinaryExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	void SetLeftAndRightBlock(HazeBaseBlock* leftJmpBlock, HazeBaseBlock* rightJmpBlock);

private:
	HazeSectionSignal m_SectionSignal;
	HazeToken m_OperatorToken;

	std::unique_ptr<ASTBase> m_LeftAST;
	std::unique_ptr<ASTBase> m_RightAST;

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
	ASTThreeExpression(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& conditionAST, 
		std::unique_ptr<ASTBase>& leftAST, std::unique_ptr<ASTBase>& rightAST);
	
	virtual ~ASTThreeExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const override { return true; }

private:
	std::unique_ptr<ASTBase> m_ConditionAST;
	std::unique_ptr<ASTBase> m_LeftAST;
	std::unique_ptr<ASTBase> m_RightAST;
};

//多行表达式
class ASTMultiExpression : public ASTBase
{
public:
	ASTMultiExpression(HazeCompiler* compiler, const SourceLocation& location, HazeSectionSignal section,
		std::vector<std::unique_ptr<ASTBase>>& expressions);

	virtual ~ASTMultiExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HazeSectionSignal m_SectionSignal;
	std::vector<std::unique_ptr<ASTBase>> m_Expressions;
};

//引用库
class ASTImportModule : public ASTBase
{
public:
	ASTImportModule(HazeCompiler* compiler, const SourceLocation& location, const HAZE_STRING& moduleName);
	
	virtual ~ASTImportModule() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HAZE_STRING m_ModuleName;
};

//Break
class ASTBreakExpression : public ASTBase
{
public:
	ASTBreakExpression(HazeCompiler* compiler, const SourceLocation& location);

	virtual ~ASTBreakExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//Continue
class ASTContinueExpression : public ASTBase
{
public:
	ASTContinueExpression(HazeCompiler* compiler, const SourceLocation& location);

	virtual ~ASTContinueExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//若
class ASTIfExpression : public ASTBase
{
public:
	ASTIfExpression(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& condition,
		std::unique_ptr<ASTBase>& ifExpression, std::unique_ptr<ASTBase>& elseExpression);
	
	virtual ~ASTIfExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

private:
	std::unique_ptr<ASTBase> m_Condition;
	std::unique_ptr<ASTBase> m_IfExpression;
	std::unique_ptr<ASTBase> m_ElseExpression;
};

//当
class ASTWhileExpression : public ASTBase
{
public:
	ASTWhileExpression(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& condition,
		std::unique_ptr<ASTBase>& multiExpression);
	
	virtual ~ASTWhileExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

private:
	std::unique_ptr<ASTBase> m_Condition;
	std::unique_ptr<ASTBase> m_MultiExpression;
};

//循环
class ASTForExpression : public ASTBase
{
public:
	ASTForExpression(HazeCompiler* compiler, const SourceLocation& location, std::unique_ptr<ASTBase>& initExpression,
		std::unique_ptr<ASTBase>& conditionExpression, std::unique_ptr<ASTBase>& stepExpression, 
		std::unique_ptr<ASTBase>& multiExpression);

	virtual	~ASTForExpression()	override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

private:
	std::unique_ptr<ASTBase> m_InitExpression;
	std::unique_ptr<ASTBase> m_ConditionExpression;
	std::unique_ptr<ASTBase> m_StepExpression;
	std::unique_ptr<ASTBase> m_MultiExpression;
};

//初始话列表
class ASTInitializeList : public ASTBase
{
public:
	ASTInitializeList(HazeCompiler* compiler, const SourceLocation& location, 
		std::vector<std::unique_ptr<ASTBase>>& initializeListExpression);

	virtual	~ASTInitializeList() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::vector<std::unique_ptr<ASTBase>> m_InitializeListExpression;
};