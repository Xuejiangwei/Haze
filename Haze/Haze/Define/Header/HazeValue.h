#pragma once

#include "HazeDefine.h"
#include <iostream>

#define CAST_TYPE(V) (x_uint32)V

//不需要引用
enum class HazeValueType : x_uint32
{
	None,

	Void,

	//基本类型 Bool -> Float64
	__BaseType_Begin,

	Bool,

	Int8,
	Int16,
	Int32,
	Int64,

	UInt8,
	UInt16,
	UInt32,
	UInt64,
	
	Float32,
	Float64,

	__BaseType_End,

	PureString,				//纯字符串, 用作操作数的Variable名字当作dynamicClass的成员名

	//引用
	Refrence,				//只作用于基本类型, 因为类和数组本来就是指针

	//函数指针
	Function,				//函数指针, 不参与GC
	
	//Object函数,
	ObjectFunction,			//Advance类型的函数

	__Advance_Begin,

	//数组类型
	Array,					//数组对象 参与GC, 不能使用Array的类型, 考虑用类封装

	//字符串
	String,					//字符串对象, 不定长, 参与GC

	Class,					//类指针对象, 参与GC

	DynamicClass,			//动态类, 不参与GC

	ObjectBase,				//基本类型的对象, 参与GC

	Hash,					//哈希对象, 参与GC

	Closure,				//闭包, 匿名函数, 参与GC

	__Advance_End,


	Enum,					//不起作用, 只用来解析时做相同类型判断

	MultiVariable,			//只能用于库函数的定义中

	DynamicClassUnknow,		//动态类成员类型
};

struct HazeValue
{
	union
	{
		bool Bool;

		x_int8 Int8;
		x_uint8 UInt8;
		x_int16 Int16;
		x_uint16 UInt16;
		x_int32 Int32;
		x_uint32 UInt32;
		x_int64 Int64;
		x_uint64 UInt64;

		x_float32 Float32;
		x_float64 Float64;

		const void* Pointer;

		union
		{
			int StringTableIndex;
			int MemorySize;
			//const HString* StringPointer;
		} Extra;
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
};

enum class InstructionOpCode : x_uint32;
enum class HazeToken : x_uint32;

x_uint32 GetSizeByHazeType(HazeValueType type);

HazeToken GetTokenByValueType(HazeValueType type);

HazeValueType GetValueTypeByToken(HazeToken token);

HazeValueType GetStrongerType(HazeValueType type1, HazeValueType type2, bool isLog = true);


// 判断类型
bool IsNoneType(HazeValueType type);
bool IsVoidType(HazeValueType type);
bool IsHazeBaseTypeAndVoid(HazeValueType type);
bool IsHazeBaseType(HazeValueType type);
bool IsAdvanceType(HazeValueType type);
bool IsIntegerType(HazeValueType type);
bool IsUnsignedIntegerType(HazeValueType type);
bool IsFloatingType(HazeValueType type);
bool IsFunctionType(HazeValueType type);
bool IsNumberType(HazeValueType type);
bool IsClassType(HazeValueType type);
bool IsEnumType(HazeValueType type);
bool IsArrayType(HazeValueType type);
bool IsDynamicClassType(HazeValueType type);
bool IsDynamicClassUnknowType(HazeValueType type);
bool IsStringType(HazeValueType type);
bool IsPureStringType(HazeValueType type);
bool IsRefrenceType(HazeValueType type);
bool IsMultiVariableTye(HazeValueType type);
bool IsObjectFunctionType(HazeValueType type);
bool IsObjectBaseType(HazeValueType type);
bool IsHashType(HazeValueType type);
bool IsClosureType(HazeValueType type);

void StringToHazeValueNumber(const HString& str, HazeValueType type, HazeValue& value);

void CalculateValueByType(HazeValueType type, InstructionOpCode typeCod, const void* source, const void* oper1, const void* oper2);

void CompareValueByType(HazeValueType type, struct HazeRegister* hazeRegister, const void* source, const void* target);
bool IsEqualByType(HazeValueType type, HazeValue v1, HazeValue v2);

size_t GetHazeCharPointerLength(const x_HChar* hChar);

const x_HChar* GetHazeValueTypeString(HazeValueType type);

void SetHazeValueByData(HazeValue& value, HazeValueType type, void* data);

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType type, const HazeValue& value);

HazeValue GetNegValue(HazeValueType type, const HazeValue& value);

bool CanCVT(HazeValueType type1, HazeValueType type2);

bool CanArray(HazeValueType type);

bool CanHash(HazeValueType type);
bool CanHashValue(HazeValueType type);

bool IsUseTemplateType(HazeValueType type);