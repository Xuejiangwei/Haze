#pragma once

#include <memory>

#include "Haze.h"

class HazeVM;
class HazeCompilerValue;
class HazeFunction;

//Base
class ASTBase
{
public:
	ASTBase();
	virtual ~ASTBase();

	virtual HazeCompilerValue* CodeGen() = 0;

protected:
	HazeValue Value;
	HazeVM* VM;
};

//Bool
class ASTVariableDefine : public ASTBase
{
public:
	ASTVariableDefine(HazeVM* VM, HazeSectionSignal Section, HazeValueType Type, HazeValue Value, std::unique_ptr<ASTBase> Expression);
	virtual ~ASTVariableDefine() override;

	bool GetValue() { return Value.BoolValue; }

	virtual HazeCompilerValue* CodeGen() override;

private:
	HazeSectionSignal SectionSignal;
	HazeValueType ValueType;
	std::unique_ptr<ASTBase> Expression;
};

//Bool
class ASTBool : public ASTBase
{
public:
	ASTBool(HazeValue V);
	virtual ~ASTBool() override;

	bool GetValue() { return Value.BoolValue; }
};

//Char
class ASTChar : public ASTBase
{
public:
	ASTChar(HazeValue V);
	virtual ~ASTChar() override;

	bool GetValue() { return Value.CharValue; }
};

//Byte
class ASTByte : public ASTBase
{
public:
	ASTByte(HazeValue V);
	virtual ~ASTByte() override;

	bool GetValue() { return Value.ByteValue; }
};

//Short
class ASTShort : public ASTBase
{
public:
	ASTShort(HazeValue V);
	virtual ~ASTShort() override;

	bool GetValue() { return Value.ShortValue; }
};

//Int
class ASTInt : public ASTBase
{
public:
	ASTInt(HazeValue V);
	virtual ~ASTInt() override;

	bool GetValue() { return Value.IntValue; }
};

//Float
class ASTFloat : public ASTBase
{
public:
	ASTFloat(HazeValue V);
	virtual ~ASTFloat() override;

	bool GetValue() { return Value.FloatValue; }
};

//Long
class ASTLong : public ASTBase
{
public:
	ASTLong(HazeValue V);
	virtual ~ASTLong() override;

	bool GetValue() { return Value.LongValue; }
};

//Double
class ASTDouble : public ASTBase
{
public:
	ASTDouble(HazeValue V);
	virtual ~ASTDouble() override;

	bool GetValue() { return Value.DoubleValue; }
};

//Unsigned byte
class ASTUnsignedByte : public ASTBase
{
public:
	ASTUnsignedByte(HazeValue V);
	virtual ~ASTUnsignedByte() override;

	bool GetValue() { return Value.UnsignedByteValue; }
};

//Unsigned short
class ASTUnsignedShort : public ASTBase
{
public:
	ASTUnsignedShort(HazeValue V);
	virtual ~ASTUnsignedShort() override;

	bool GetValue() { return Value.UnsignedShortValue; }
};

//Unsigned int
class ASTUnsignedInt : public ASTBase
{
public:
	ASTUnsignedInt(HazeValue V);
	virtual ~ASTUnsignedInt() override;

	bool GetValue() { return Value.UnsignedIntValue; }
};

//Unsigned long
class ASTUnsignedLong : public ASTBase
{
public:
	ASTUnsignedLong(HazeValue V);
	virtual ~ASTUnsignedLong() override;

	bool GetValue() { return Value.UnsignedLongValue; }
};
