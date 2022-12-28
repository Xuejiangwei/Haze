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
		WSS >> Value.Value.Float;
		break;
	case HazeValueType::Double:
		WSS >> Value.Value.Double;
		break;
	case HazeValueType::Byte:
	{
		short Temp;
		WSS >> Temp;
		memcpy(&Value.Value.Byte, &Temp, sizeof(Value.Value.Byte));
	}
	break;
	case HazeValueType::Short:
		WSS >> Value.Value.Short;
		break;
	case HazeValueType::Int:
		WSS >> Value.Value.Int;
		break;
	case HazeValueType::Long:
		WSS >> Value.Value.Long;
		break;
	case HazeValueType::UnsignedByte:
	{
		unsigned short Temp;
		WSS >> Temp;
		memcpy(&Value.Value.UnsignedByte, &Temp, sizeof(Value.Value.Byte));
	}
	break;
	case HazeValueType::UnsignedShort:
		WSS >> Value.Value.UnsignedShort;
		break;
	case HazeValueType::UnsignedInt:
		WSS >> Value.Value.UnsignedInt;
		break;
	case HazeValueType::UnsignedLong:
		WSS >> Value.Value.UnsignedLong;
		break;
	default:
		break;
	}
}
