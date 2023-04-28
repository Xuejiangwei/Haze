#pragma once

#include "HazeDefine.h"
#include <iostream>

enum class HazeValueType : uint32
{
	Void,
	Bool,

	Char,

	Int,
	Float,
	Long,
	Double,

	UnsignedInt,
	UnsignedLong,

	Array,

	PointerBase,
	PointerClass,
	PointerPointer,

	ReferenceBase,
	ReferenceClass,

	Class,
	Function,

	MultiVariable,
};

struct HazeValue
{
	//HazeValueType Type;

	union
	{
		bool Bool;

		HAZE_CHAR Char;
		int Int;
		float Float;
		long long Long;
		double Double;

		unsigned int UnsignedInt;
		unsigned long long UnsignedLong;

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

bool IsHazeDefaultTypeAndVoid(HazeValueType Type);

bool IsHazeDefaultType(HazeValueType Type);

bool IsIntegerType(HazeValueType Type);

bool IsNumberType(HazeValueType Type);

void StringToHazeValueNumber(const HAZE_STRING& Str, HazeValueType Type, HazeValue& Value);

void CalculateValueByType(HazeValueType Type, InstructionOpCode TypeCode, const void* Source, const void* Target);

void CompareValueByType(HazeValueType Type, struct HazeRegister* Register, const void* Source, const void* Target);

size_t GetHazeCharPointerLength(const HAZE_CHAR* Char);

const HAZE_CHAR* GetHazeValueTypeString(HazeValueType Type);

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType Type, HazeValue& Value);