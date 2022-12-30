#include "Haze.h"

HazeValueType GetValueTypeByToken(HazeToken Token)
{
	static std::unordered_map<HazeToken, HazeValueType> MapHashTable =
	{
		{ HazeToken::Bool, HazeValueType::Bool },
		{ HazeToken::Char, HazeValueType::Char },
		{ HazeToken::Byte, HazeValueType::Byte },
		{ HazeToken::Short, HazeValueType::Short },
		{ HazeToken::Int, HazeValueType::Int },
		{ HazeToken::Float, HazeValueType::Float },
		{ HazeToken::Long, HazeValueType::Long },
		{ HazeToken::Double, HazeValueType::Double },
		{ HazeToken::UnsignedByte, HazeValueType::UnsignedByte },
		{ HazeToken::UnsignedShort, HazeValueType::UnsignedShort },
		{ HazeToken::UnsignedInt, HazeValueType::UnsignedInt },
		{ HazeToken::UnsignedLong, HazeValueType::UnsignedLong}
	};

	auto it = MapHashTable.find(Token);
	if (it != MapHashTable.end())
	{
		return it->second;
	}

	return HazeValueType::Null;
}

unsigned int GetSize(HazeValueType Type)
{
	switch (Type)
	{
	case Bool:
	case Char:
	case Byte:
	case UnsignedByte:
		return 1;
	case Short:
	case UnsignedShort:
		return 2;
	case Int:
	case Float:
	case UnsignedInt:
		return 4;
	case Long:
	case Double:
	case UnsignedLong:
		return 8;
	default:
		break;
	}
	return 0;
}