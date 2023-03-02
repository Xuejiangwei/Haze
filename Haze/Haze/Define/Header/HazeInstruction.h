#pragma once

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
	Register,
	Temp,
};

enum class InstructionOpCode : unsigned int
{
	NONE,					//指令码id	目标数据类型(InstructionDataType)		目标内存地址		源数据类型(InstructionDataType)		源数据内存
	MOV,					//1			需要									需要				需要									需要
	ADD,					//2			需要									需要				需要									需要
	SUB,					//3			需要									需要				需要									需要
	MUL,					//4			需要									需要				需要									需要
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

	PUSH,					//17		操作数类型
	POP,

	CALL,
	RET,

	//字符串
	Concat,//连接两个字符串
	GetChar,
	SetChar,
};

//Jmp 等跳转label,需要在第一遍遍历源文件时将所有label及其后面的相邻一条指令的数组索引的收集(注意重复的报错处理，所有的指令都要存在一个数组里面)，
//在第二遍生成字节码时，Jmp label替换成 Jmp label对应指令的索引

//函数及函数调用处理同Jmp处理

struct InstructionData
{
	InstructionScopeType Scope;
	HAZE_STRING Name;
	HazeValueType Type;
	unsigned int Index = -1;
};