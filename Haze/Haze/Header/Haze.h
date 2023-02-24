﻿#pragma once

#include <unordered_map>
#include <sstream>
#include <string>

#define HAZE_STRING_STREAM std::wstringstream
#define HAZE_OFSTREAM std::wofstream
#define HAZE_STRING std::wstring
#define HAZE_CHAR wchar_t

#define HAZE_TEXT(S) L##S

#define HAZE_MAIN_FUNCTION_TEXT HAZE_TEXT("主函数")

#define HAZE_I_CODE_ENABLE			1
#define HAZE_OP_CODE_ENABLE		1

enum class HazeSectionSignal : unsigned __int8
{
	Global,
	Function,
	Class,
};

enum class HazeToken : unsigned int
{
	None,
	Identifier,
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

	Function,

	MainFunction,

	Class,
	ClassData,

	True,
	False,

	Add,
	Sub,
	Mul,
	Div,
	Mod,

	And,
	Or,
	Not,

	LeftMove,
	RightMove,

	Assign,
	Equal,
	NotEqual,
	Greater,
	GreaterEqual,
	Less,
	LessEqual,

	Inc,
	Dec,

	LeftParentheses,
	RightParentheses,

	Comma,

	LeftBrace,
	RightBrace,

	If,
	Else,

	For,
	ForStep,

	Break,
	Continue,
	Return,

	While,

	Cast,

	Reference,

	Define,

	ImportModule,

	//NoMatch
	Number,
	String,
};

enum HazeValueType : unsigned int
{
	Null,
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
		bool Bool;
		char Char;

		__int8 Byte;
		short Short;
		int Int;
		float Float;
		long long Long;
		double Double;

		unsigned __int8 UnsignedByte;
		unsigned short UnsignedShort;
		unsigned int UnsignedInt;
		unsigned long long UnsignedLong;
	} Value;

public:
	HazeValue& operator =(const HazeValue& V)
	{
		memcpy(this, &V, sizeof(V));
		return *this;
	}

	HazeValue& operator =(int64_t V)
	{
		memcpy(&this->Value, &V, sizeof(V));
		return *this;
	}

	HazeValue& operator +=(const HazeValue& V)
	{
		return *this;
	}
};

//字节码文件头部数据格式定义(模仿linux程序结构 堆区、栈区、全局数据区、只读数据区等)
enum HazeOpCodeFileFormat
{
	None,
	GlobalData,
	GlobalFunction,
};

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

enum class InstructionDataType : unsigned int
{
	None,
	Global,
	Local,
	Constant,
	Register,
};

enum class InstructionOpCode : unsigned int
{
	NONE,					//指令码id	目标数据类型(InstructionDataType)		目标内存地址		源数据类型(InstructionDataType)		源数据内存
	MOV,					//1			需要									需要				需要									需要
	ADD,					//2			需要									需要				需要									需要
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

	//字符串
	Concat,//连接两个字符串
	GetChar,
	SetChar,
};

//Jmp 等跳转label,需要在第一遍遍历源文件时将所有label及其后面的相邻一条指令的数组索引的收集(注意重复的报错处理，所有的指令都要存在一个数组里面)，
//在第二遍生成字节码时，Jmp label替换成 Jmp label对应指令的索引

//函数及函数调用处理同Jmp处理

extern HazeValueType GetValueTypeByToken(HazeToken Token);

extern void StringToHazeValueNumber(const HAZE_STRING& Str, HazeValue& Value);

extern unsigned int GetSize(HazeValueType Type);

extern bool IsNumber(const HAZE_STRING& Str);

/*
 * Generate file header string
 */
extern const HAZE_CHAR* GetGlobalDataHeaderString();

extern const HAZE_CHAR* GetStringTableHeaderString();

extern const HAZE_CHAR* GetFucntionTableHeaderString();

extern const HAZE_CHAR* GetFunctionLabelHeader();

extern const HAZE_CHAR* GetFunctionParamHeader();

extern const HAZE_CHAR* GetFunctionStartHeader();

extern const HAZE_CHAR* GetFunctionEndHeader();

template <typename T>
T StringToInt(HAZE_STRING& String)
{
	HAZE_STRING_STREAM WSS;
	WSS << String;

	T Ret;
	WSS >> Ret;

	return Ret;
}

/*
 * Instruction string
 */
extern InstructionOpCode GetInstructionByString(const HAZE_STRING& String);

using HazeDefineType = std::pair<HazeToken, HAZE_STRING>;			//Token, Token是identifier时的类型名
using HazeDefineVariable = std::pair<HazeDefineType, HAZE_STRING>;	//HazeDefineType, 定义的变量名

