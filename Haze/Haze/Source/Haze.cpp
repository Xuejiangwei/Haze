#include <regex>


#include "Haze.h"


#define HEADER_STRING_GLOBAL_DATA HAZE_TEXT("GlobalDataTable")
#define HEADER_STRING_STRING_TABLE HAZE_TEXT("StringTable")
#define HEADER_STRING_FUNCTION_TABLE HAZE_TEXT("FunctionTable")

#define FUNCTION_LABEL_HEADER HAZE_TEXT("Function")
#define FUNCTION_PARAM_HEADER HAZE_TEXT("Param")
#define FUNCTION_START_HEADER HAZE_TEXT("Start")
#define FUNCTION_END_HEADER HAZE_TEXT("End")

static std::unordered_map<HAZE_STRING, InstructionOpCode> HashMap_String2Code = {
	{HAZE_TEXT("MOV"), InstructionOpCode::MOV },
	{HAZE_TEXT("ADD"), InstructionOpCode::ADD },
	{HAZE_TEXT("SUB"), InstructionOpCode::SUB },
	{HAZE_TEXT("MUL"), InstructionOpCode::MUL },
	{HAZE_TEXT("DIV"), InstructionOpCode::DIV },
	{HAZE_TEXT("MOD"), InstructionOpCode::MOD },
	{HAZE_TEXT("EXP"), InstructionOpCode::EXP },
	{HAZE_TEXT("NEG"), InstructionOpCode::NEG },
	{HAZE_TEXT("INC"), InstructionOpCode::INC },
	{HAZE_TEXT("DEC"), InstructionOpCode::DEC },

	{HAZE_TEXT("AND"), InstructionOpCode::AND },
	{HAZE_TEXT("OR"), InstructionOpCode::OR },
	{HAZE_TEXT("NOT"), InstructionOpCode::NOT },
	{HAZE_TEXT("XOR"), InstructionOpCode::XOR },
	{HAZE_TEXT("SHL"), InstructionOpCode::SHL },
	{HAZE_TEXT("SHR"), InstructionOpCode::SHR },
	
	{HAZE_TEXT("PUSH"), InstructionOpCode::PUSH },
	{HAZE_TEXT("POP"), InstructionOpCode::POP },
	
	{HAZE_TEXT("CALL"), InstructionOpCode::CALL },
};

bool IsNumber(const HAZE_STRING& Str)
{
	std::wregex Pattern(HAZE_TEXT("-[0-9]+(.[0-9]+)?|[0-9]+(.[0-9]+)?"));
	return std::regex_match(Str, Pattern);
}

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

void StringToHazeValueNumber(const HAZE_STRING& Str, HazeValue& Value)
{
	HAZE_STRING_STREAM WSS;
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

const HAZE_CHAR* GetGlobalDataHeaderString()
{
	return HEADER_STRING_GLOBAL_DATA;
}

const HAZE_CHAR* GetStringTableHeaderString()
{
	return HEADER_STRING_STRING_TABLE;
}

const HAZE_CHAR* GetFucntionTableHeaderString()
{
	return HEADER_STRING_FUNCTION_TABLE;
}

const HAZE_CHAR* GetFunctionLabelHeader()
{
	return FUNCTION_LABEL_HEADER;
}

const HAZE_CHAR* GetFunctionParamHeader()
{
	return FUNCTION_PARAM_HEADER;
}

const HAZE_CHAR* GetFunctionStartHeader()
{
	return FUNCTION_START_HEADER;
}

const HAZE_CHAR* GetFunctionEndHeader()
{
	return FUNCTION_END_HEADER;
}

InstructionOpCode GetInstructionByString(const HAZE_STRING& String)
{
	auto iter = HashMap_String2Code.find(String);
	if (iter != HashMap_String2Code.end())
	{
		return iter->second;
	}

	return InstructionOpCode::NONE;
}
