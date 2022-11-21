#pragma once

#include <memory>

#include "Haze.h"

class HazeVM;
class HazeCompilerValue;

//Base
class ASTBase
{
public:
	ASTBase(HazeVM* VM);
	ASTBase(HazeVM* VM, HazeValue Value);
	virtual ~ASTBase();

	virtual HazeCompilerValue* CodeGen() { return  nullptr; }

protected:
	HazeVM* VM;
	HazeValue Value;
};

//����
class ASTBool : public ASTBase
{
public:
	ASTBool(HazeVM* VM, HazeValue V);
	virtual ~ASTBool() override;

	virtual HazeCompilerValue* CodeGen() override;
};

//����
class ASTNumber : public ASTBase
{
public:
	ASTNumber(HazeVM* VM, HazeValue V);
	virtual ~ASTNumber() override;

	virtual HazeCompilerValue* CodeGen() override;
};

//��������
class ASTVariableDefine : public ASTBase
{
public:
	ASTVariableDefine(HazeVM* VM, HazeSectionSignal Section, HazeValueType Type, HAZE_STRING& Name, std::unique_ptr<ASTBase>& Expression);
	virtual ~ASTVariableDefine() override;

	virtual HazeCompilerValue* CodeGen() override;

private:
	HazeSectionSignal SectionSignal;
	HazeValueType Type;
	HAZE_STRING Name;
	std::unique_ptr<ASTBase> Expression;
};
