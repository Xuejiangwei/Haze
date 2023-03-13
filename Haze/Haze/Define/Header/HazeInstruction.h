﻿#pragma once

//操作数数据顺序 ValueType Name Scope

/*  指令字节码说明
	uint8		uint8		uint8		uint64		(uint8		uint64)		...(uint8		uint64)
	指令字节id	操作数个数	操作数类型	操作数值		(操作数类型	操作数值)	...(操作数类型	操作数值)
*/

enum class InstructionOpCodeType : unsigned char
{
	Memory,
	Bool,
	Char,

	Byte,
	Short,
	Int,
	Float,
	Long,
	Double,

	UnsignedByte,
	UnsignedShort,
	UnsignedInt,
	UnsignedLong,
};

enum class InstructionScopeType : unsigned int
{
	None,
	Global,
	Local,
	Constant,
	String,
	Register,
	Temp,

	Address,
};

enum class InstructionOpCode : unsigned int
{
	NONE,
	MOV,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	INC,
	DEC,

	AND,
	OR,
	NOT,
	XOR,
	SHL,
	SHR,

	PUSH,
	POP,

	CALL,
	RET,
};

//Jmp 等跳转label,需要在第一遍遍历源文件时将所有label及其后面的相邻一条指令的数组索引的收集(注意重复的报错处理，所有的指令都要存在一个数组里面)，
//在第二遍生成字节码时，Jmp label替换成 Jmp label对应指令的索引

//函数及函数调用处理同Jmp处理

enum class InstructionFunctionType : unsigned int
{
	HazeFunction,
	StdLibFunction,
};



struct InstructionData
{
	InstructionScopeType Scope;
	HAZE_STRING Name;
	HazeValueType Type;

	union
	{
		int Index;
		int Offset;
		int FunctionCallParamNum;
	} Extra;
};

struct Instruction
{
	InstructionOpCode InsCode;
	std::vector<InstructionData> Operator;
};

struct FunctionDescData
{
	InstructionFunctionType Type;
	unsigned int InstructionStartAddress;
};



struct FunctionData
{
	using StdLibFunctionCall = void(*)(HAZE_STD_CALL_PARAM);

	HazeValueType Type;
	std::vector<std::pair<HAZE_STRING, HazeValue>> Vector_Param;
	unsigned int InstructionNum;

	union
	{
		FunctionDescData FunctionDescData;
		StdLibFunctionCall StdLibFunction;
	} Extra;
};