#pragma once

#include <iostream>

enum class HazeValueType : unsigned int
{
	Null,
	Bool,
	Char,

	Short,
	Int,
	Float,
	Long,
	Double,

	UnsignedChar,
	UnsignedShort,
	UnsignedInt,
	UnsignedLong,

	Class,
	Function,
};

struct HazeValue
{
	HazeValueType Type;

	union
	{
		bool Bool;
		char Char;

		__int8 Byte;
		short Short;
		int Int;
		float Float;
		long long Long;
		double Double;

		unsigned __int8 UnsignedByte;
		unsigned short UnsignedShort;
		unsigned int UnsignedInt;
		unsigned long long UnsignedLong;
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

	HazeValue& operator +=(const HazeValue& V)
	{
		return *this;
	}
};
