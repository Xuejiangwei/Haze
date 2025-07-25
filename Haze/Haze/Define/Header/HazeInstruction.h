#pragma once

//操作数数据顺序 ValueType Name Scope

/*  指令字节码说明
	uint8		uint8		uint8		uint64		(uint8		uint64)		...(uint8		uint64)
	指令字节id	操作数个数	操作数类型	操作数值		(操作数类型	操作数值)	...(操作数类型	操作数值)
*/
enum class InstructionOpCodeType : x_uint8
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

#define CAST_SCOPE(V) (x_uint32)V
#define IS_SCOPE_GLOBAL(V) V == HazeVariableScope::Global
#define IS_SCOPE_LOCAL(V) V == HazeVariableScope::Local
#define IS_SCOPE_IGNORE(V) V == HazeVariableScope::Ignore
enum class HazeVariableScope : x_uint32
{
	None,
	Global,
	Local,
	Static,

	Ignore,		
};

#define CAST_DESC(V) (x_uint32)V
enum class HazeDataDesc : x_uint32
{
	None,
	/*Global,
	Local,*/
	Constant,
	ConstantString,

	RegisterBegin,
	RegisterRet,
	//RegisterNew,
	RegisterCmp,
	RegisterTemp,
	RegisterEnd,

	Address,
	FunctionAddress,
	FunctionDynamicAddress,
	FunctionObjectAddress,

	Element,

	ClassThis,
	Class,
	ClassMember_Local_Public,
	ClassMember_Local_Private,
	ClassFunction_Local_Public,
	ClassFunction_Local_Private,

	Initlist,

	NullPtr,

	CallFunctionModule,
};

enum class InstructionOpCode : x_uint32
{
#define HAZE_OP_CODE_DEFINE(OP_CODE) OP_CODE,
	#include "HazeOpCodeTemplate"
#undef HAZE_OP_CODE_DEFINE
};

//Jmp 等跳转label,需要在第一遍遍历源文件时将所有label及其后面的相邻一条指令的数组索引的收集(注意重复的报错处理，所有的指令都要存在一个数组里面)，
//在第二遍生成字节码时，Jmp label替换成 Jmp label对应指令的索引

//函数及函数调用处理同Jmp处理

enum class InstructionFunctionType : int
{
	HazeFunction,
	StaticLibFunction,
	DLLLibFunction,
};

enum class InstructionAddressType : x_uint8
{
	Global,
	Local,

	FunctionAddress,
	FunctionDynamicAddress,
	FunctionObjectAddress,

	Constant,
	NullPtr,
	ConstantString,

	PureString,

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

	struct ObjectFunctionCall
	{
		int ParamNum;
		int Index;
	};

	struct BlockJmp
	{
		int StartAddress;
		int InstructionNum;
	};

	struct NewSign
	{
		union
		{
			x_uint32 TemplateCount;
			x_uint32 ArrayDimension;
		};
	};

	union Extra
	{
		int Index;
		AddressData Address;
		FunctionCall Call;
		ObjectFunctionCall ObjectCall;
		BlockJmp Jmp;
		HazeValue RuntimeDynamicValue;

		x_uint32 Line;
		NewSign SignData;
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

	InstructionData& operator =(const InstructionData& value)
	{
		memcpy(this, &value, sizeof(value));
		return *this;
	}
};

struct Instruction
{
	InstructionOpCode InsCode;
	V_Array<InstructionData> Operator;
};

struct FunctionDescData
{
	using StdLibFunctionCall = void(*)(HAZE_STD_CALL_PARAM);

	InstructionFunctionType Type;

	union
	{
		x_uint64 InstructionStartAddress;
		StdLibFunctionCall StdLibFunction;
	};

	x_uint32 StartLine;
	x_uint32 EndLine;
};

struct ModuleData
{
	HString Name;
	HString Path;
	Pair<x_uint32, x_uint32> GlobalDataIndex;
	Pair<x_uint32, x_uint32> StringIndex;
	Pair<x_uint32, x_uint32> ClassIndex;
	Pair<x_uint32, x_uint32> FunctionIndex;
	HazeLibraryType LibType;

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
	HString Name;
	x_uint32 Size;
	x_uint32 TypeId;
	V_Array<HazeVariableData> Members;
	HashMap<HString, x_uint32> Functions;
	V_Array<x_uint32> InheritClasses;
};

struct FunctionData
{
	HazeVariableType Type;
	V_Array<HazeDefineVariable> Params;
	V_Array<HazeVariableData> Variables;
	V_Array<HazeTempRegisterData> TempRegisters;
	V_Array<Pair<int, int>> RefVariables;
	x_uint32 InstructionNum;

	FunctionDescData FunctionDescData;
};

struct HazeRegister
{
	V_Array<char> Data;
	HazeVariableType Type;
};

//struct HazeJmpData
//{
//	int CachePC;					//执行完需要跳转回的pc
//	//int BlockInstructionNum;		//跳转的block指令剩余数
//	//int SkipNum;					//跳转回PC时，因为比较为true时，没有block，所以需要加上为true时的block的指令个数
//};

//struct HazFrameFunctionData
//{
//	V_Array<HazeDefineVariable*> LocalParams;
//};

bool IsRegisterDesc(HazeDataDesc desc);
bool IsClassMember(HazeDataDesc desc);
bool IsConstant(HazeDataDesc desc);

bool IsJmpOpCode(InstructionOpCode code);
bool IsArithmeticOpCode(InstructionOpCode opcode);
bool IsComparisonOpCode(InstructionOpCode opcode);
bool IsCallOpCode(InstructionOpCode opcode);
bool IsMovOpCode(InstructionOpCode opcode);

const x_HChar* GetInstructionString(InstructionOpCode code);

InstructionOpCode GetInstructionByString(const HString& str);
