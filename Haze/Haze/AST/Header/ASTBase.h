#pragma once

#include <vector>
#include <memory>

#include "Haze.h"

class HazeVM;
class HazeCompilerValue;

//Base
class ASTBase
{
	friend class ASTClass;

public:
	ASTBase(HazeVM* VM);
	ASTBase(HazeVM* VM, const HazeDefineVariable& DefVar);
	virtual ~ASTBase();

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() { return  nullptr; }

	virtual const HAZE_CHAR* GetName() { return HAZE_TEXT(""); }

	virtual bool IsBlock() const { return false; }

protected:
	HazeVM* VM;
	HazeValue Value;
	HazeDefineVariable DefineVariable;
};

//����
class ASTBool : public ASTBase
{
public:
	ASTBool(HazeVM* VM, const HazeValue& InValue);
	virtual ~ASTBool() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//����
class ASTNumber : public ASTBase
{
public:
	ASTNumber(HazeVM* VM, HazeValueType Type, const HazeValue& InValue);
	virtual ~ASTNumber() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//�ַ���
class ASTStringText : public ASTBase
{
public:
	ASTStringText(HazeVM* VM, HAZE_STRING& Text);
	virtual ~ASTStringText() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HAZE_STRING Text;
};

//����
class ASTIdentifier : public ASTBase
{
public:
	ASTIdentifier(HazeVM* VM, HazeSectionSignal Section, HAZE_STRING& Name, HAZE_STRING* MemberName, std::vector<std::unique_ptr<ASTBase>>& ArrayIndexExpression);
	virtual ~ASTIdentifier() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual const HAZE_CHAR* GetName() { return DefineVariable.Name.c_str(); }

private:
	HazeSectionSignal SectionSignal;
	HAZE_STRING ClassMemberName;
	std::vector<std::unique_ptr<ASTBase>> ArrayIndexExpression;
};

//��������
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

//��������
class ASTVariableDefine : public ASTBase
{
public:

	ASTVariableDefine(HazeVM* VM, HazeSectionSignal Section, const HazeDefineVariable& DefineVariable, std::unique_ptr<ASTBase> Expression, std::vector<std::unique_ptr<ASTBase>> ArraySize = {}, int PointerLevel = 0, std::vector<HazeDefineType>* Vector_ParamType = nullptr);
	virtual ~ASTVariableDefine() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HazeSectionSignal SectionSignal;
	std::unique_ptr<ASTBase> Expression;
	std::vector<std::unique_ptr<ASTBase>> ArraySize;
	int PointerLevel;

	std::vector<HazeDefineType> Vector_PointerFunctionParamType;
};

//����
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

//��õ�ַ
class ASTGetAddress : public ASTBase
{
public:
	ASTGetAddress(HazeVM* VM, std::unique_ptr<ASTBase>& Expression);
	virtual ~ASTGetAddress() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Expression;
};

//Parse pointer value
class ASTPointerValue : public ASTBase
{
public:
	ASTPointerValue(HazeVM* VM, std::unique_ptr<ASTBase>& Expression, int Level, std::unique_ptr<ASTBase> AssignExpression = nullptr);
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
	ASTNeg(HazeVM* VM, std::unique_ptr<ASTBase>& Expression);
	virtual ~ASTNeg() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::unique_ptr<ASTBase> Expression;
};

//Inc
class ASTInc : public ASTBase
{
public:
	ASTInc(HazeVM* VM, std::unique_ptr<ASTBase>& Expression, bool IsPreInc);
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
	ASTDec(HazeVM* VMM, std::unique_ptr<ASTBase>& Expression, bool IsPreDec);
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
	ASTOperetorAssign(HazeVM* VM, HazeToken Token, std::unique_ptr<ASTBase>& Expression);
	virtual ~ASTOperetorAssign() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HazeToken Token;
	std::unique_ptr<ASTBase> Expression;
};

//��Ԫ���ʽ
class ASTBinaryExpression : public ASTBase
{
	friend class ASTIfExpression;
	friend class ASTWhileExpression;
	friend class ASTForExpression;
	friend class ASTThreeExpression;
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

//��Ŀ���ʽ
class ASTThreeExpression : public ASTBase
{
	friend class ASTIfExpression;
	friend class ASTWhileExpression;
	friend class ASTForExpression;
public:
	ASTThreeExpression(HazeVM* VM, std::unique_ptr<ASTBase>& ConditionAST, std::unique_ptr<ASTBase>& LeftAST, std::unique_ptr<ASTBase>& RightAST);
	virtual ~ASTThreeExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const override { return true; }

private:
	std::unique_ptr<ASTBase> ConditionAST;
	std::unique_ptr<ASTBase> LeftAST;
	std::unique_ptr<ASTBase> RightAST;

};

//���б��ʽ
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

//���ÿ�
class ASTImportModule : public ASTBase
{
public:
	ASTImportModule(HazeVM* VM, const HAZE_STRING& ModuleName);
	virtual ~ASTImportModule() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	HAZE_STRING ModuleName;
};

//Break
class ASTBreakExpression : public ASTBase
{
public:
	ASTBreakExpression(HazeVM* VM);
	virtual ~ASTBreakExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//Continue
class ASTContinueExpression : public ASTBase
{
public:
	ASTContinueExpression(HazeVM* VM);
	virtual ~ASTContinueExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;
};

//��
class ASTIfExpression : public ASTBase
{
public:
	ASTIfExpression(HazeVM* VM, std::unique_ptr<ASTBase>& Condition, std::unique_ptr<ASTBase>& IfExpression, std::unique_ptr<ASTBase>& ElseExpression);
	virtual ~ASTIfExpression() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

	virtual bool IsBlock() const { return true; }

private:
	std::unique_ptr<ASTBase> Condition;
	std::unique_ptr<ASTBase> IfExpression;
	std::unique_ptr<ASTBase> ElseExpression;
};

//��
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

//ѭ��
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

//��ʼ���б�
class ASTInitializeList : public ASTBase
{
public:
	ASTInitializeList(HazeVM* VM, std::vector<std::unique_ptr<ASTBase>>& InitializeListExpression);
	virtual	~ASTInitializeList() override;

	virtual std::shared_ptr<HazeCompilerValue> CodeGen() override;

private:
	std::vector<std::unique_ptr<ASTBase>> InitializeListExpression;
};