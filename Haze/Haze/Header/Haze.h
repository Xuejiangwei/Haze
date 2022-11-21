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
	} Value;
};

//�ֽ����ļ�ͷ�����ݸ�ʽ����(ģ��linux����ṹ ������ջ����ȫ����������ֻ����������)
enum HazeOpCodeFileFormat
{
	None,
	GlobalData,
	GlobalFunction,
};

enum class InstructionOpCode : unsigned int
{
	NONE,					//ָ����id	Ŀ���ڴ��ַ		Դ��������	Դ�����ڴ�
	MOV,					//1			��Ҫ				��Ҫ			��Ҫ
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
