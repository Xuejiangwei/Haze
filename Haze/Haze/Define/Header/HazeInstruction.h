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
	UnsignedByte,

	Short,
	UnsignedShort,

	Int,
	UnsignedInt,

	Float,

	Long,
	UnsignedLong,

	Double,
};

#define CAST_SCOPE(V) (uint32)V
#define IS_SCOPE_GLOBAL(V) V == HazeVariableScope::Global
#define IS_SCOPE_LOCAL(V) V == HazeVariableScope::Local
#define IS_SCOPE_TEMP(V) V == HazeVariableScope::Temp
enum class HazeVariableScope : uint32
{
	None,
	Global,
	Local,
	Static,

	Temp,		//解析时生成中间变量
	Ignore,
};

#define CAST_DESC(V) (uint32)V
enum class HazeDataDesc : uint32
{
	None,
	/*Global,
	Local,*/
	Constant,
	ConstantString,

	RegisterBegin,
	RegisterRet,
	RegisterNew,
	RegisterCmp,
	RegisterTemp,
	RegisterEnd,

	Address,
	FunctionAddress,

	ClassThis,
	ClassPointer,
	Class,
	ClassMember_Local_Public,
	ClassMember_Local_Private,
	ClassMember_Local_Protected,
	ClassFunction_Local_Public,
	ClassFunction_Local_Private,
	ClassFunction_Local_Protected,

	Initlist,
	ArrayElement,

	NullPtr,

	CallFunctionModule,
};

enum class InstructionOpCode : uint32
{
	NONE,
	MOV,
	MOVPV,
	MOVTOPV,
	LEA,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,

	NEG,

	NOT,

	INC,
	DEC,

	BIT_AND,
	BIT_OR,
	BIT_NEG,
	BIT_XOR,
	SHL,
	SHR,

	PUSH,
	POP,

	CALL,
	RET,

	NEW,

	CMP,
	JMP,
	JNE,		//不等于
	JNG,		//不大于
	JNL,		//不小于
	JE,			//等于
	JG,			//大于
	JL,			//小于

	ADD_ASSIGN,
	SUB_ASSIGN,
	MUL_ASSIGN,
	DIV_ASSIGN,
	MOD_ASSIGN,
	BIT_AND_ASSIGN,
	BIT_OR_ASSIGN,
	BIT_XOR_ASSIGN,
	SHL_ASSIGN,
	SHR_ASSIGN,

	LINE,			//调试用
};

//Jmp 等跳转label,需要在第一遍遍历源文件时将所有label及其后面的相邻一条指令的数组索引的收集(注意重复的报错处理，所有的指令都要存在一个数组里面)，
//在第二遍生成字节码时，Jmp label替换成 Jmp label对应指令的索引

//函数及函数调用处理同Jmp处理

enum class InstructionFunctionType : uint32
{
	HazeFunction,
	StdLibFunction,
	DLLLibFunction,
};

enum class InstructionAddressType : uint8
{
	Global,
	Global_Base_Offset,
	Global_BasePointer_Offset,

	Local,
	Local_Base_Offset,
	Local_BasePointer_Offset,

	FunctionAddress,

	Constant,
	NullPtr,
	ConstantString,

	Register,
};

struct InstructionData
{
	HazeDefineVariable Variable;
	HazeVariableScope Scope;
	HazeDataDesc Desc;
	InstructionAddressType AddressType;

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

	union Extra
	{
		int Index;
		AddressData Address;
		FunctionCall Call;
		BlockJmp Jmp;
		void* Pointer;

		uint32 Line;

		Extra()
		{
		}
	} Extra;

	InstructionData() : Variable(), Desc(HazeDataDesc::None)
	{
		AddressType = InstructionAddressType::Local;
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
	using StdLibFunctionCall = void(*)(HAZE_STD_CALL_PARAM);

	InstructionFunctionType Type;

	union
	{
		uint32 InstructionStartAddress;
		StdLibFunctionCall StdLibFunction;
	};

	uint32 StartLine;
	uint32 EndLine;
};

struct ModuleData
{
	HAZE_STRING Name;
	std::pair<uint32, uint32> GlobalDataIndex;
	std::pair<uint32, uint32> StringIndex;
	std::pair<uint32, uint32> ClassIndex;
	std::pair<uint32, uint32> FunctionIndex;

	ModuleData()
	{
		Name.clear();
		GlobalDataIndex = { 0, 0 };
		StringIndex = { 0, 0 };
		ClassIndex = { 0, 0 };
		FunctionIndex = { 0, 0 };
	}
};

struct ClassData
{
	HAZE_STRING Name;
	unsigned int Size;
	std::vector<HazeVariableData> Members;
};

struct FunctionData
{
	HazeValueType Type;
	std::vector<HazeDefineVariable> Params;
	std::vector<HazeVariableData> Variables;
	uint32 InstructionNum;

	FunctionDescData FunctionDescData;
};

struct HazeRegister
{
	std::vector<char> Data;
	HazeDefineType Type;
};

//struct HazeJmpData
//{
//	int CachePC;					//执行完需要跳转回的pc
//	//int BlockInstructionNum;		//跳转的block指令剩余数
//	//int SkipNum;					//跳转回PC时，因为比较为true时，没有block，所以需要加上为true时的block的指令个数
//};

struct HazFrameFunctionData
{
	std::vector<HazeDefineVariable*> LocalParams;
};

bool IsRegisterDesc(HazeDataDesc scope);

bool IsJmpOpCode(InstructionOpCode code);

bool IsClassMember(HazeDataDesc scope);

const HAZE_CHAR* GetInstructionString(InstructionOpCode code);

InstructionOpCode GetInstructionByString(const HAZE_STRING& str);