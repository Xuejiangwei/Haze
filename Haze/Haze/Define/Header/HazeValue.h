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

uint32 GetSizeByHazeType(HazeValueType m_Type);

HazeValueType GetValueTypeByToken(HazeToken m_Token);

HazeValueType GetStrongerType(HazeValueType Type1, HazeValueType Type2);

bool IsVoidType(HazeValueType type);

bool IsHazeDefaultTypeAndVoid(HazeValueType type);

bool IsHazeDefaultType(HazeValueType type);

bool IsIntegerType(HazeValueType type);

bool IsPointerType(HazeValueType type);

bool IsPointerFunction(HazeValueType type);

bool IsNumberType(HazeValueType type);

bool IsClassType(HazeValueType type);

bool IsArrayType(HazeValueType type);

bool IsReferenceType(HazeValueType type);

void StringToHazeValueNumber(const HAZE_STRING& Str, HazeValueType m_Type, HazeValue& Value);

void OperatorValueByType(HazeValueType m_Type, InstructionOpCode TypeCode, const void* Target);

void CalculateValueByType(HazeValueType m_Type, InstructionOpCode TypeCode, const void* Source, const void* Target);

void CompareValueByType(HazeValueType m_Type, struct HazeRegister* Register, const void* Source, const void* Target);

size_t GetHazeCharPointerLength(const HAZE_CHAR* Char);

const HAZE_CHAR* GetHazeValueTypeString(HazeValueType m_Type);

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType m_Type, const HazeValue& Value);

HazeValue GetNegValue(HazeValueType m_Type, const HazeValue& Value);