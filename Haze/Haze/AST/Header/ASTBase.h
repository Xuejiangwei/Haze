#pragma once

#include <vector>
#include <memory>

#include "Haze.h"

class HazeVM;
class HazeCompilerValue;

//Base
class ASTBase
{
public:
	ASTBase(HazeVM* VM);
	ASTBase(HazeVM* VM, const HazeDefineVariable& DefVar);
	ASTBase(HazeVM* VM, const HazeValue& Value);
	virtual ~ASTBase();

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() { return  nullptr; }

	virtual const HAZE_CHAR* GetName() { return HAZE_TEXT(""); }

protected:
	HazeVM* VM;
	HazeValue Value;
	HazeDefineVariable DefineVariable;
};

//布尔
class ASTBool : public ASTBase
{
public:
	ASTBool(HazeVM* VM, const HazeValue& Value);
	virtual ~ASTBool() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//数字
class ASTNumber : public ASTBase
{
public:
	ASTNumber(HazeVM* VM, const HazeValue& Value);
	virtual ~ASTNumber() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//字符串
class ASTStringText : public ASTBase
{
public:
	ASTStringText(HazeVM* VM, HAZE_STRING& Text);
	virtual ~ASTStringText() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HAZE_STRING Text;
};


//变量
class ASTIdentifier : public ASTBase
{
public:
	ASTIdentifier(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HAZE_STRING* MemberName = nullptr);
	virtual ~ASTIdentifier() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual const HAZE_CHAR* GetName() { return Name.c_str(); }

private:
	HazeSectionSignal SectionSignal;
	HAZE_STRING Name;
	HAZE_STRING ClassMemberName;
};

//函数调用
class ASTFunctionCall : public ASTBase
{
public:
	ASTFunctionCall(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, std::vector<std::unique_ptr<ASTBase>>& FunctionParam);
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
	friend class ASTClass;

	ASTVariableDefine(HazeVM* VM, HazeSectionSignal Section, const HazeDefineVariable& DefineVariable, std::unique_ptr<ASTBase> Expression);
	virtual ~ASTVariableDefine() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HazeSectionSignal SectionSignal;
	std::unique_ptr<ASTBase> Expression;
};

//返回
class ASTReturn : public ASTBase
{
public:
	ASTReturn(HazeVM* VM, std::unique_ptr<ASTBase>& Expression);
	virtual ~ASTReturn() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Expression;
};

//New
class ASTNew : public ASTBase
{
public:
	ASTNew(HazeVM* VM, const HazeDefineVariable& Define);
	virtual ~ASTNew() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//二元表达式
class ASTBinaryExpression : public ASTBase
{
	friend class ASTIfExpression;
	friend class ASTWhileExpression;
	friend class ASTForExpression;
public:
	ASTBinaryExpression(HazeVM* VM, HazeSectionSignal Section, HazeToken OperatorToken, std::unique_ptr<ASTBase>& LeftAST, std::unique_ptr<ASTBase>& RightAST);
	virtual ~ASTBinaryExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;


private:
	HazeSectionSignal SectionSignal;
	HazeToken OperatorToken;

	std::unique_ptr<ASTBase> LeftAST;
	std::unique_ptr<ASTBase> RightAST;

};

//多行表达式
class ASTMultiExpression : public ASTBase
{
public:
	ASTMultiExpression(HazeVM* VM, HazeSectionSignal Section, std::vector<std::unique_ptr<ASTBase>>& VectorExpression);
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
	ASTImportModule(HazeVM* VM, const HAZE_STRING& ModuleName);
	virtual ~ASTImportModule() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HAZE_STRING ModuleName;
};

//若
class ASTIfExpression : public ASTBase
{
public:
	ASTIfExpression(HazeVM* VM, std::unique_ptr<ASTBase>& Condition, std::unique_ptr<ASTBase>& IfExpression, std::unique_ptr<ASTBase>& ElseExpression);
	virtual ~ASTIfExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Condition;
	std::unique_ptr<ASTBase> IfExpression;
	std::unique_ptr<ASTBase> ElseExpression;
};

//当
class ASTWhileExpression : public ASTBase
{
public:
	ASTWhileExpression(HazeVM* VM, std::unique_ptr<ASTBase>& Condition, std::unique_ptr<ASTBase>& MultiExpression);
	virtual ~ASTWhileExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Condition;
	std::unique_ptr<ASTBase> MultiExpression;
};

//循环
class ASTForExpression : public ASTBase
{
public:
	ASTForExpression(HazeVM* VM, std::unique_ptr<ASTBase>& InitExpression, std::unique_ptr<ASTBase>& ConditionExpression, std::unique_ptr<ASTBase>& StepExpression,
		std::unique_ptr<ASTBase>& MultiExpression);
	virtual	~ASTForExpression()	override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> InitExpression;
	std::unique_ptr<ASTBase> ConditionExpression;
	std::unique_ptr<ASTBase> StepExpression;
	std::unique_ptr<ASTBase> MultiExpression;
};

