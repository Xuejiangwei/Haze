#pragma once

//操作数数据顺序 ValueType Name Scope

/*  指令字节码说明
	uint8		uint8		uint8		uint64		(uint8		uint64)		...(uint8		uint64)
	指令字节id	操作数个数	操作数类型	操作数值		(操作数类型	操作数值)	...(操作数类型	操作数值)
*/

enum class InstructionOpCodeType : uint8
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

enum class HazeDataDesc : uint32
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
	RegisterCmp,
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

enum class InstructionOpCode : uint32
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

	CMP,
	JMP,
	JMPL,
	JNE,		//不等于
	JNG,		//不大于
	JNL,		//不小于
	JE,			//等于
	JG,			//大于
	JL,			//小于

	JMPOUT,			//JmpOut 跳出block
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
	GlobalDataIndex,
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

	struct BlockJmp
	{
		int StartAddress;
		int InstructionNum;
	};

	InstructionAddressType AddressType;
	union Extra
	{
		int Index;
		AddressData Address;
		FunctionCall Call;
		BlockJmp Jmp;
		void* Pointer;

		Extra()
		{
		}
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
	std::vector<HazeLocalVariable> Vector_Variable;
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

struct HazeJmpData
{
	int CachePC;					//执行完需要跳转回的pc
	//int BlockInstructionNum;		//跳转的block指令剩余数
	//int SkipNum;					//跳转回PC时，因为比较为true时，没有block，所以需要加上为true时的block的指令个数
};

struct HazFrameFunctionData
{
	std::vector<HazeDefineVariable*> Vector_LocalParam;
};

bool IsRegisterScope(HazeDataDesc Scope);

bool IsJmpOpCode(InstructionOpCode Code);

const HAZE_CHAR* GetInstructionString(InstructionOpCode Code);

InstructionOpCode GetInstructionByString(const HAZE_STRING& String);
