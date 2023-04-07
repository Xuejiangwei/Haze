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

enum class HazeDataDesc : unsigned int
{
	None,
	Global,
	Local,
	Constant,
	ConstantString,
	Temp,

	RegisterBegin,
	RegisterRet,
	RegisterNew,
	RegisterEnd,

	Address,
	ClassThis,
	Class,
	ClassMember_Local_Public,
	ClassMember_Local_Private,
	ClassMember_Local_Protected,
	ClassFunction_Local_Public,
	ClassFunction_Local_Private,
	ClassFunction_Local_Protected,
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

	NEW,
};

//Jmp 等跳转label,需要在第一遍遍历源文件时将所有label及其后面的相邻一条指令的数组索引的收集(注意重复的报错处理，所有的指令都要存在一个数组里面)，
//在第二遍生成字节码时，Jmp label替换成 Jmp label对应指令的索引

//函数及函数调用处理同Jmp处理

enum class InstructionFunctionType : uint32
{
	HazeFunction,
	StdLibFunction,
};

enum class InstructionAddressType : uint8
{
	Direct,
	Index,
	Address_Offset,
	Pointer_Offset,
};

struct InstructionData
{
	HazeDefineVariable Variable;
	HazeDataDesc Scope;

	struct AddressData
	{
		int BaseAddress;
		int Offset = 0;
	};

	struct FunctionCall
	{
		int ParamNum;
		int ParamByteSize;
	};

	InstructionAddressType AddressType;
	union Extra
	{
		int Index;
		AddressData Address;
		FunctionCall Call;
		void* Pointer;

		Extra()
		{}
	} Extra;

	InstructionData() : Scope(HazeDataDesc::None), Variable()
	{
		AddressType = InstructionAddressType::Direct;
		memset(&Extra, 0, sizeof(Extra));
	}

	~InstructionData()
	{
	}
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

struct ClassData
{
	HAZE_STRING Name;
	unsigned int Size;
	std::vector<HazeDefineVariable> Vector_Member;
};

struct FunctionData
{
	using StdLibFunctionCall = void(*)(HAZE_STD_CALL_PARAM);

	HazeValueType Type;
	std::vector<HazeDefineVariable> Vector_Param;
	unsigned int InstructionNum;

	union
	{
		FunctionDescData FunctionDescData;
		StdLibFunctionCall StdLibFunction;
	} Extra;
};

struct HazeRegister
{
	std::vector<char> Data;
	HazeDefineType Type;
};

bool IsRegisterScope(HazeDataDesc Scope);

const HAZE_CHAR* GetInstructionString(InstructionOpCode Code);

InstructionOpCode GetInstructionByString(const HAZE_STRING& String);
