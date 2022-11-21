#pragma once

#include <regex>
#include <sstream>

#include "Haze.h"

bool IsNumber(const HAZE_STRING& Str)
{
	std::wregex Pattern(HAZE_TEXT("-[0-9]+(.[0-9]+)?|[0-9]+(.[0-9]+)?"));
	return std::regex_match(Str, Pattern);
}

void StringToNumber(const HAZE_STRING& Str, HazeValue& Value)
{
	std::wstringstream WSS;
	WSS << Str;

	switch (Value.Type)
	{
	case HazeValueType::Float:
		WSS >> Value.Value.FloatValue;
		break;
	case HazeValueType::Double:
		WSS >> Value.Value.DoubleValue;
		break;
	case HazeValueType::Byte:
	{
		short Temp;
		WSS >> Temp;
		memcpy(&Value.Value.ByteValue, &Temp, sizeof(Value.Value.ByteValue));
	}
	break;
	case HazeValueType::Short:
		WSS >> Value.Value.ShortValue;
		break;
	case HazeValueType::Int:
		WSS >> Value.Value.IntValue;
		break;
	case HazeValueType::Long:
		WSS >> Value.Value.LongValue;
		break;
	case HazeValueType::UnsignedByte:
	{
		unsigned short Temp;
		WSS >> Temp;
		memcpy(&Value.Value.UnsignedByteValue, &Temp, sizeof(Value.Value.ByteValue));
	}
	break;
	case HazeValueType::UnsignedShort:
		WSS >> Value.Value.UnsignedShortValue;
		break;
	case HazeValueType::UnsignedInt:
		WSS >> Value.Value.UnsignedIntValue;
		break;
	case HazeValueType::UnsignedLong:
		WSS >> Value.Value.UnsignedLongValue;
		break;
	default:
		break;
	}
}
