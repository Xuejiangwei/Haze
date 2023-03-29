#pragma once

#include "HazeDefine.h"
#include <iostream>

enum class HazeValueType : unsigned int
{
	Void,
	Bool,

	Char,

	Short,
	Int,
	Float,
	Long,
	Double,

	UnsignedShort,
	UnsignedInt,
	UnsignedLong,

	PointerBase,
	PointerClass,

	Class,
	Function,

	MultiVar,
};

struct HazeValue
{
	HazeValueType Type;

	union
	{
		bool Bool;

		short Short;
		int Int;
		float Float;
		long long Long;
		double Double;

		unsigned __int8 UnsignedByte;
		unsigned short UnsignedShort;
		unsigned int UnsignedInt;
		unsigned long long UnsignedLong;

		const void* Pointer;

		union
		{
			//int StringTableIndex;
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

enum class InstructionOpCode : unsigned int;

bool IsHazeDefaultType(HazeValueType Type);

bool IsNumberType(HazeValueType Type);

void CalculateValueByType(HazeValueType Type, InstructionOpCode TypeCode, const char* Source, const char* Target);

size_t GetHazeCharPointerLength(const HAZE_CHAR* Char);