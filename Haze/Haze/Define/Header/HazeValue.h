#pragma once

#include "HazeDefine.h"
#include <iostream>

#define CAST_TYPE(V) (uint32)V

//不需要引用
enum class HazeValueType : uint32
{
	None,

	Void,
	


	//基本类型 Bool -> Double
	__BaseType_Begin,

	Bool,

	Int8,
	UInt8,

	//Char,	 不需要字符类型

	Int16,
	UInt16,

	Int32,
	UInt32,

	Int64,
	UInt64,
	
	Float32,
	Float64,

	__BaseType_End,



	//引用
	Refrence,				//只作用于基本类型，因为类和数组本来就是指针

	//函数指针
	Function,				//函数指针, 不参与GC
	


	__Advance_Begin,

	//数组类型
	Array,					//数组对象 参与GC, 定长

	//字符串
	String,					//字符串对象, 不定长, 参与GC

	Class,					//类指针对象, 参与GC

	__Advance_End,



	Enum,					//不起作用, 只用来解析时做相同类型判断

	MultiVariable,			//只能用于库函数的定义中
};

struct HazeValue
{
	union
	{
		bool Bool;

		int8 Int8;
		uint8 UInt8;
		int16 Int16;
		uint16 UInt16;
		int32 Int32;
		uint32 UInt32;
		int64 Int64;
		uint64 UInt64;

		float32 Float32;
		float64 Float64;

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

enum class InstructionOpCode : uint32;
enum class HazeToken : uint32;

uint32 GetSizeByHazeType(HazeValueType type);

HazeToken GetTokenByValueType(HazeValueType type);

HazeValueType GetValueTypeByToken(HazeToken token);

HazeValueType GetValueTypeByName(const HString& str);

HazeValueType GetStrongerType(HazeValueType type1, HazeValueType type2, bool isLog = true);


// 判断类型
bool IsNoneType(HazeValueType type);
bool IsVoidType(HazeValueType type);
bool IsHazeBaseTypeAndVoid(HazeValueType type);
bool IsHazeBaseType(HazeValueType type);
bool IsAdvanceType(HazeValueType type);
bool IsIntegerType(HazeValueType type);
bool IsFloatingType(HazeValueType type);
bool IsClassType(HazeValueType type);
bool IsFunctionType(HazeValueType type);
bool IsNumberType(HazeValueType type);
bool IsClassType(HazeValueType type);
bool IsEnumType(HazeValueType type);
bool IsArrayType(HazeValueType type);
bool IsStringType(HazeValueType type);
bool IsRefrenceType(HazeValueType type);
bool IsMultiVariableTye(HazeValueType type);


void StringToHazeValueNumber(const HString& str, HazeValueType type, HazeValue& value);

void OperatorValueByType(HazeValueType type, InstructionOpCode typeCode, const void* target);

void CalculateValueByType(HazeValueType type, InstructionOpCode typeCode, const void* source, const void* target);

void CompareValueByType(HazeValueType type, struct HazeRegister* hazeRegister, const void* source, const void* target);

size_t GetHazeCharPointerLength(const HChar* hChar);

const HChar* GetHazeValueTypeString(HazeValueType type);

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType type, const HazeValue& value);

HazeValue GetNegValue(HazeValueType type, const HazeValue& value);

bool CanCVT(HazeValueType type1, HazeValueType type2);