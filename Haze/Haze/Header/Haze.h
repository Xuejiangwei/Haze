#pragma once

#include <string>

#define HAZE_STRING std::wstring
#define HAZE_CHAR wchar_t

#define HAZE_TEXT(S) L##S

enum class HazeSectionSignal : unsigned __int8
{
	Global,
	Local,
	Class,
};

enum HazeValueType : unsigned int
{
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

	Class,
	Function,
};

struct HazeValue
{
	HazeValueType Type;

	union
	{
		bool BoolValue;
		char CharValue;

		__int8 ByteValue;
		short ShortValue;
		int IntValue;
		float FloatValue;
		long long LongValue;
		double DoubleValue;

		unsigned __int8 UnsignedByteValue;
		unsigned short UnsignedShortValue;
		unsigned int UnsignedIntValue;
		unsigned long long UnsignedLongValue;
	};
};

//字节码文件头部数据格式定义(模仿linux程序结构 堆区、栈区、全局数据区、只读数据区等)
enum HazeOpCodeFileFormat
{
	None,
	GlobalData,
	GlobalFunction,
};

enum class InstructionOpCode : unsigned int
{
	NONE,					//指令码id	目标内存地址		源数据类型	源数据内存
	MOV,					//1			需要				需要			需要
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	EXP,
	NEG,
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
};
