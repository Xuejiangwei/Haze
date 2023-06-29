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
	ASTBase(HazeCompiler* Compiler, const SourceLocation& Location);
	ASTBase(HazeCompiler* Compiler, const SourceLocation& Location, const HazeDefineVariable& DefVar);
	virtual ~ASTBase();

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() { return  nullptr; }

	virtual const HAZE_CHAR* GetName() { return HAZE_TEXT(""); }

	virtual bool IsBlock() const { return false; }

	const HazeDefineVariable& GetDefine() const { return DefineVariable; }

protected:
	HazeCompiler* Compiler;
	HazeValue Value;
	HazeDefineVariable DefineVariable;

	SourceLocation Location;
};

//布尔
class ASTBool : public ASTBase
{
public:
	ASTBool(HazeCompiler* Compiler, const SourceLocation& Location, const HazeValue& InValue);
	virtual ~ASTBool() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//数字
class ASTNumber : public ASTBase
{
public:
	ASTNumber(HazeCompiler* Compiler, const SourceLocation& Location, HazeValueType Type, const HazeValue& InValue);
	virtual ~ASTNumber() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//字符串
class ASTStringText : public ASTBase
{
public:
	ASTStringText(HazeCompiler* Compiler, const SourceLocation& Location, HAZE_STRING& Text);
	virtual ~ASTStringText() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HAZE_STRING Text;
};

//变量
class ASTIdentifier : public ASTBase
{
public:
	ASTIdentifier(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, HAZE_STRING& Name, std::vector<std::unique_ptr<ASTBase>>& ArrayIndexExpression);
	virtual ~ASTIdentifier() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual const HAZE_CHAR* GetName() { return DefineVariable.Name.c_str(); }

private:
	HazeSectionSignal SectionSignal;
	std::vector<std::unique_ptr<ASTBase>> ArrayIndexExpression;
};

//函数调用
class ASTFunctionCall : public ASTBase
{
public:
	ASTFunctionCall(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, HAZE_STRING& Name, std::vector<std::unique_ptr<ASTBase>>& FunctionParam);
	virtual ~ASTFunctionCall() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual const HAZE_CHAR* GetName() { return Name.c_str(); }

private:
	HazeSectionSignal SectionSignal;
	HAZE_STRING Name;

	std::vector<std::unique_ptr<ASTBase>> FunctionParam;
};

//变量定义
class ASTVariableDefine : public ASTBase
{
public:

	ASTVariableDefine(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, const HazeDefineVariable& DefineVariable, std::unique_ptr<ASTBase> Expression, std::vector<std::unique_ptr<ASTBase>> ArraySize = {}, int PointerLevel = 0, std::vector<HazeDefineType>* Vector_ParamType = nullptr);
	virtual ~ASTVariableDefine() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual const HAZE_CHAR* GetName() { return DefineVariable.Name.c_str(); }

private:
	HazeSectionSignal SectionSignal;
	std::unique_ptr<ASTBase> Expression;
	std::vector<std::unique_ptr<ASTBase>> ArraySize;
	int PointerLevel;

	std::vector<HazeDefineType> Vector_PointerFunctionParamType;
};

//返回
class ASTReturn : public ASTBase
{
public:
	ASTReturn(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression);
	virtual ~ASTReturn() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Expression;
};

//New
class ASTNew : public ASTBase
{
public:
	ASTNew(HazeCompiler* Compiler, const SourceLocation& Location, const HazeDefineVariable& Define);
	virtual ~ASTNew() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//获得地址
class ASTGetAddress : public ASTBase
{
public:
	ASTGetAddress(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression);
	virtual ~ASTGetAddress() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Expression;
};

//Parse pointer value
class ASTPointerValue : public ASTBase
{
public:
	ASTPointerValue(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression, int Level, std::unique_ptr<ASTBase> AssignExpression = nullptr);
	virtual ~ASTPointerValue() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Expression;
	std::unique_ptr<ASTBase> AssignExpression;
	int Level;
};

//Parse pointer value
class ASTNeg : public ASTBase
{
public:
	ASTNeg(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression, bool IsNumberNeg);
	virtual ~ASTNeg() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Expression;
	bool IsNumberNeg;
};

//Null pointer
class ASTNullPtr : public ASTBase
{
public:
	ASTNullPtr(HazeCompiler* Compiler, const SourceLocation& Location);
	virtual ~ASTNullPtr() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	void SetDefineType(const HazeDefineType& Type);
};

//Inc
class ASTInc : public ASTBase
{
public:
	ASTInc(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression, bool IsPreInc);
	virtual ~ASTInc() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Expression;
	bool IsPreInc;
};

//Dec
class ASTDec : public ASTBase
{
public:
	ASTDec(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Expression, bool IsPreDec);
	virtual ~ASTDec() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Expression;
	bool IsPreDec;
};

//Operator
class ASTOperetorAssign : public ASTBase
{
public:
	ASTOperetorAssign(HazeCompiler* Compiler, const SourceLocation& Location, HazeToken Token, std::unique_ptr<ASTBase>& Expression);
	virtual ~ASTOperetorAssign() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HazeToken Token;
	std::unique_ptr<ASTBase> Expression;
};

//二元表达式
class ASTBinaryExpression : public ASTBase
{
	friend class ASTIfExpression;
	friend class ASTWhileExpression;
	friend class ASTForExpression;
	friend class ASTThreeExpression;
public:
	ASTBinaryExpression(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, HazeToken OperatorToken, std::unique_ptr<ASTBase>& LeftAST, std::unique_ptr<ASTBase>& RightAST);
	virtual ~ASTBinaryExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	void SetLeftAndRightBlock(HazeBaseBlock* LeftJmpBlock, HazeBaseBlock* RightJmpBlock);

private:
	HazeSectionSignal SectionSignal;
	HazeToken OperatorToken;

	std::unique_ptr<ASTBase> LeftAST;
	std::unique_ptr<ASTBase> RightAST;

	HazeBaseBlock* LeftBlock;
	HazeBaseBlock* RightBlock;
	HazeBaseBlock* DafaultBlock;
};

//三目表达式
class ASTThreeExpression : public ASTBase
{
	friend class ASTIfExpression;
	friend class ASTWhileExpression;
	friend class ASTForExpression;
public:
	ASTThreeExpression(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& ConditionAST, std::unique_ptr<ASTBase>& LeftAST, std::unique_ptr<ASTBase>& RightAST);
	virtual ~ASTThreeExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const override { return true; }

private:
	std::unique_ptr<ASTBase> ConditionAST;
	std::unique_ptr<ASTBase> LeftAST;
	std::unique_ptr<ASTBase> RightAST;
};

//多行表达式
class ASTMultiExpression : public ASTBase
{
public:
	ASTMultiExpression(HazeCompiler* Compiler, const SourceLocation& Location, HazeSectionSignal Section, std::vector<std::unique_ptr<ASTBase>>& VectorExpression);
	virtual ~ASTMultiExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HazeSectionSignal SectionSignal;
	std::vector<std::unique_ptr<ASTBase>> VectorExpression;
};

//引用库
class ASTImportModule : public ASTBase
{
public:
	ASTImportModule(HazeCompiler* Compiler, const SourceLocation& Location, const HAZE_STRING& ModuleName);
	virtual ~ASTImportModule() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HAZE_STRING ModuleName;
};

//Break
class ASTBreakExpression : public ASTBase
{
public:
	ASTBreakExpression(HazeCompiler* Compiler, const SourceLocation& Location);
	virtual ~ASTBreakExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//Continue
class ASTContinueExpression : public ASTBase
{
public:
	ASTContinueExpression(HazeCompiler* Compiler, const SourceLocation& Location);
	virtual ~ASTContinueExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//若
class ASTIfExpression : public ASTBase
{
public:
	ASTIfExpression(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Condition, std::unique_ptr<ASTBase>& IfExpression, std::unique_ptr<ASTBase>& ElseExpression);
	virtual ~ASTIfExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

private:
	std::unique_ptr<ASTBase> Condition;
	std::unique_ptr<ASTBase> IfExpression;
	std::unique_ptr<ASTBase> ElseExpression;
};

//当
class ASTWhileExpression : public ASTBase
{
public:
	ASTWhileExpression(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& Condition, std::unique_ptr<ASTBase>& MultiExpression);
	virtual ~ASTWhileExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

private:
	std::unique_ptr<ASTBase> Condition;
	std::unique_ptr<ASTBase> MultiExpression;
};

//循环
class ASTForExpression : public ASTBase
{
public:
	ASTForExpression(HazeCompiler* Compiler, const SourceLocation& Location, std::unique_ptr<ASTBase>& InitExpression, std::unique_ptr<ASTBase>& ConditionExpression, std::unique_ptr<ASTBase>& StepExpression,
		std::unique_ptr<ASTBase>& MultiExpression);
	virtual	~ASTForExpression()	override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

private:
	std::unique_ptr<ASTBase> InitExpression;
	std::unique_ptr<ASTBase> ConditionExpression;
	std::unique_ptr<ASTBase> StepExpression;
	std::unique_ptr<ASTBase> MultiExpression;
};

//初始话列表
class ASTInitializeList : public ASTBase
{
public:
	ASTInitializeList(HazeCompiler* Compiler, const SourceLocation& Location, std::vector<std::unique_ptr<ASTBase>>& InitializeListExpression);
	virtual	~ASTInitializeList() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::vector<std::unique_ptr<ASTBase>> InitializeListExpression;
};