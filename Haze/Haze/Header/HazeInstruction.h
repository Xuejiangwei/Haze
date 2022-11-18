#pragma once

class HazeInstruction
{
public:
	enum class OpCode : unsigned __int8 //×Ö½ÚÂë
	{
	};

	struct OpData
	{
		int Type;
		union
		{
			bool BoolValue;
			char CharValue;

			__int8 ByteValue;
			short ShortValue;
			int IntValue;
			float FloatValue;
			long long LongValue;
			double DoubleValue;

			unsigned __int8 UnsignedByteValue;
			unsigned short UnsignedShortValue;
			unsigned int UnsignedIntValue;
			unsigned long long UnsignedLongValue;
		};

		int OffsetIndex;		//Æ«ÒÆÁ¿Ë÷Òý
	};

	struct Instruction
	{
		OpCode Opcode;
		int OpCount;
	};

public:
	HazeInstruction();
	~HazeInstruction();

private:
};
