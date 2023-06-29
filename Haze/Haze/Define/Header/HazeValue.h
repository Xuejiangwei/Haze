#pragma once

#include "HazeDefine.h"
#include <iostream>

#define CAST_TYPE(V) (uint32)V

enum class HazeValueType : uint32
{
	Void,
	Bool,

	Byte,
	UnsignedByte,

	Char,

	Short,
	UnsignedShort,

	Int,
	Float,
	Long,
	Double,

	UnsignedInt,
	UnsignedLong,

	Array,

	PointerBase,
	PointerClass,
	PointerFunction,
	PointerArray,
	PointerPointer,

	ReferenceBase,
	ReferenceClass,

	Class,
	Function,

	MultiVariable,
};

struct HazeValue
{
	union
	{
		bool Bool;

		hbyte Byte;
		uhbyte UnsignedByte;

		HAZE_CHAR Char;

		short Short;
		ushort UnsignedShort;

		int Int;
		float Float;
		long long Long;
		double Double;

		uint32 UnsignedInt;
		uint64 UnsignedLong;

		const void* Pointer;

		union
		{
			int StringTableIndex;
			int MemorySize;
			//const HAZE_STRING* StringPointer;
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

uint32 GetSizeByHazeType(HazeValueType Type);

HazeValueType GetValueTypeByToken(HazeToken Token);

HazeValueType GetStrongerType(HazeValueType Type1, HazeValueType Type2);

bool IsVoidType(HazeValueType Type);

bool IsHazeDefaultTypeAndVoid(HazeValueType Type);

bool IsHazeDefaultType(HazeValueType Type);

bool IsIntegerType(HazeValueType Type);

bool IsPointerType(HazeValueType Type);

bool IsPointerFunction(HazeValueType Type);

bool IsNumberType(HazeValueType Type);

bool IsClassType(HazeValueType Type);

void StringToHazeValueNumber(const HAZE_STRING& Str, HazeValueType Type, HazeValue& Value);

void OperatorValueByType(HazeValueType Type, InstructionOpCode TypeCode, const void* Target);

void CalculateValueByType(HazeValueType Type, InstructionOpCode TypeCode, const void* Source, const void* Target);

void CompareValueByType(HazeValueType Type, struct HazeRegister* Register, const void* Source, const void* Target);

size_t GetHazeCharPointerLength(const HAZE_CHAR* Char);

const HAZE_CHAR* GetHazeValueTypeString(HazeValueType Type);

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType Type, const HazeValue& Value);

HazeValue GetNegValue(HazeValueType Type, const HazeValue& Value);