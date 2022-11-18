#include "ASTBase.h"
#include "HazeVM.h"
#include "HazeCompiler.h"
#include "HazeCompilerValue.h"

//Base
ASTBase::ASTBase()
{
}

ASTBase::~ASTBase()
{
}

ASTVariableDefine::ASTVariableDefine(HazeVM* VM, HazeSectionSignal Section, HazeValueType Type, HazeValue Value, std::unique_ptr<ASTBase> Expression)
	: SectionSignal(Section), ValueType(Type), Expression(std::move(Expression))
{
	this->VM = VM;
	this->Value = Value;
}

ASTVariableDefine::~ASTVariableDefine()
{
}

HazeCompilerValue* ASTVariableDefine::CodeGen()
{
	HazeCompilerValue* Value = nullptr;
	std::unique_ptr<HazeCompiler>& Compiler = VM->GetCompiler();
	if (SectionSignal == HazeSectionSignal::Global)
	{
		//生成全局变量字节码
		Value = Compiler->GenGlobalVariable();
	}
	else if (SectionSignal == HazeSectionSignal::Local)
	{
		//生成局部变量字节码
		Value = Compiler->GenLocalVariable();
	}

	HazeCompilerValue* ExprValue = nullptr;
	if (Expression)
	{
		ExprValue = Expression->CodeGen();
	}

	Value->StoreValue(ExprValue);

	return Value;
}

//Bool
ASTBool::ASTBool(HazeValue V) //: ASTBase(V)
{
}

ASTBool::~ASTBool()
{
}

//Char
ASTChar::ASTChar(HazeValue V) //: ASTBase(V)
{
}

ASTChar::~ASTChar()
{
}

//Byte
ASTByte::ASTByte(HazeValue V) //: ASTBase(V)
{
}

ASTByte::~ASTByte()
{
}

//Short
ASTShort::ASTShort(HazeValue V) //: ASTBase(V)
{
}

ASTShort::~ASTShort()
{
}

//Int
ASTInt::ASTInt(HazeValue V) //: ASTBase(V)
{
}
ASTInt::~ASTInt()
{
}

//Float
ASTFloat::ASTFloat(HazeValue V) //: ASTBase(V)
{
}

ASTFloat::~ASTFloat()
{
}

//Long
ASTLong::ASTLong(HazeValue V) //: ASTBase(V)
{
}

ASTLong::~ASTLong()
{
}

//Double
ASTDouble::ASTDouble(HazeValue V) : ASTBase(V)
{
}

ASTDouble::~ASTDouble()
{
}

//Unsigned byte
ASTUnsignedByte::ASTUnsignedByte(HazeValue V) : ASTBase(V)
{
}

ASTUnsignedByte::~ASTUnsignedByte()
{
}

//Unsigned short
ASTUnsignedShort::ASTUnsignedShort(HazeValue V) : ASTBase(V)
{
}

ASTUnsignedShort::~ASTUnsignedShort()
{
}

//Unsigned int
ASTUnsignedInt::ASTUnsignedInt(HazeValue V) : ASTBase(V)
{
}
ASTUnsignedInt::~ASTUnsignedInt()
{
}

//Unsigned long
ASTUnsignedLong::ASTUnsignedLong(HazeValue V) : ASTBase(V)
{
}

ASTUnsignedLong::~ASTUnsignedLong()
{
}