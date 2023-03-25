#include <regex>
#include <Windows.h>
#include <set>

#include "Haze.h"


#define HEADER_STRING_GLOBAL_DATA HAZE_TEXT("GlobalDataTable")
#define HEADER_STRING_STRING_TABLE HAZE_TEXT("StringTable")
#define HEADER_STRING_CLASS_TABLE HAZE_TEXT("ClassTable")
#define HEADER_STRING_FUNCTION_TABLE HAZE_TEXT("FunctionTable")

#define CLASS_LABEL_HEADER HAZE_TEXT("Class")
#define FUNCTION_LABEL_HEADER HAZE_TEXT("Function")
#define FUNCTION_PARAM_HEADER HAZE_TEXT("Param")
#define FUNCTION_START_HEADER HAZE_TEXT("Start")
#define FUNCTION_END_HEADER HAZE_TEXT("End")

static std::unordered_map<HAZE_STRING, InstructionOpCode> HashMap_String2Code = 
{
	{HAZE_TEXT("MOV"), InstructionOpCode::MOV },
	{HAZE_TEXT("ADD"), InstructionOpCode::ADD },
	{HAZE_TEXT("SUB"), InstructionOpCode::SUB },
	{HAZE_TEXT("MUL"), InstructionOpCode::MUL },
	{HAZE_TEXT("DIV"), InstructionOpCode::DIV },
	{HAZE_TEXT("MOD"), InstructionOpCode::MOD },
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
	{HAZE_TEXT("RET"), InstructionOpCode::RET },

	{HAZE_TEXT("NEW"), InstructionOpCode::NEW },
};

bool IsNumber(const HAZE_STRING& Str)
{
	std::wregex Pattern(HAZE_TEXT("-[0-9]+(.[0-9]+)?|[0-9]+(.[0-9]+)?"));
	return std::regex_match(Str, Pattern);
}

HazeValueType GetNumberDefaultType(const HAZE_STRING& Str)
{
	std::wregex Pattern(HAZE_TEXT("^[0-9]+(.[0-9])+$"));
	bool IsFloat = std::regex_match(Str, Pattern);
	if (IsFloat)
	{
		return HazeValueType::Float;
	}
	else
	{
		return HazeValueType::Int;
	}
	return HazeValueType::Int;
}

HazeValueType GetStrongerType(HazeValueType Type1, HazeValueType Type2)
{
	static std::unordered_map<HazeValueType, std::set<HazeValueType>> HashMap_Table =
	{
		{ HazeValueType::Short, { HazeValueType::Int, HazeValueType::Long, HazeValueType::Float, HazeValueType::Double } },
		{ HazeValueType::Int, { HazeValueType::Long, HazeValueType::Float, HazeValueType::Double } },
		{ HazeValueType::UnsignedShort, { HazeValueType::UnsignedInt, HazeValueType::UnsignedLong } },
		{ HazeValueType::UnsignedInt, { HazeValueType::UnsignedLong } },
		{ HazeValueType::Float, { HazeValueType::Double} }
	};

	if (Type1 == Type2)
	{
		return Type1;
	}
	else
	{
		auto it1 = HashMap_Table.find(Type1);
		auto it2 = HashMap_Table.find(Type2);
		if (it1 != HashMap_Table.end() && it1->second.find(Type2) != it1->second.end())
		{
			return Type2;
		}
		else if (it2 != HashMap_Table.end() && it2->second.find(Type1) != it2->second.end())
		{
			return Type1;
		}
	}

	static_assert(true, "Not find stronger type!");

	return HazeValueType::Void;
}

HazeValueType GetValueTypeByToken(HazeToken Token)
{
	static std::unordered_map<HazeToken, HazeValueType> MapHashTable =
	{
		{ HazeToken::Void, HazeValueType::Void },
		{ HazeToken::Bool, HazeValueType::Bool },
		{ HazeToken::Short, HazeValueType::Short },
		{ HazeToken::Int, HazeValueType::Int },
		{ HazeToken::Float, HazeValueType::Float },
		{ HazeToken::Long, HazeValueType::Long },
		{ HazeToken::Double, HazeValueType::Double },
		{ HazeToken::UnsignedShort, HazeValueType::UnsignedShort },
		{ HazeToken::UnsignedInt, HazeValueType::UnsignedInt },
		{ HazeToken::UnsignedLong, HazeValueType::UnsignedLong},
		{ HazeToken::CustomClass, HazeValueType::Class},

		{ HazeToken::MultiVariable, HazeValueType::MultiVar},
	};

	auto it = MapHashTable.find(Token);
	if (it != MapHashTable.end())
	{
		return it->second;
	}

	return HazeValueType::Void;
}

unsigned int GetSizeByType(HazeValueType Type)
{
	switch (Type)
	{
	case HazeValueType::Bool:
		return 1;
	case HazeValueType::Short:
	case HazeValueType::UnsignedShort:
		return 2;
	case HazeValueType::Int:
	case HazeValueType::Float:
	case HazeValueType::UnsignedInt:
		return 4;
	case HazeValueType::Long:
	case HazeValueType::Double:
	case HazeValueType::UnsignedLong:
	case HazeValueType::Pointer:
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

const HAZE_CHAR* GetClassTableHeaderString()
{
	return HEADER_STRING_CLASS_TABLE;
}

const HAZE_CHAR* GetClassLabelHeader()
{
	return CLASS_LABEL_HEADER;
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

const HAZE_CHAR* GetInstructionString(InstructionOpCode Code)
{
	static std::unordered_map<InstructionOpCode, const HAZE_CHAR*> HashMap_Code2String;

	if (HashMap_Code2String.size() <= 0)
	{
		for (auto& iter : HashMap_String2Code)
		{
			HashMap_Code2String[iter.second] = iter.first.c_str();
		}
	}

	auto iter = HashMap_Code2String.find(Code);
	if (iter != HashMap_Code2String.end())
	{
		return iter->second;
	}

	return HAZE_TEXT("None");
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

HAZE_STRING String2WString(std::string& str)
{
	std::wstring result;
#ifdef WIN32

	//获取缓冲区大小，并申请空间，缓冲区大小按字符计算  
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	//多字节编码转换成宽字节编码  
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';             //添加字符串结尾  
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;

#endif // WIN32

	return result;
}

std::string WString2String(std::wstring& wstr)
{
	std::string result;

#ifdef WIN32

	//获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的  
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	char* buffer = new char[len + 1];
	//宽字节编码转换成多字节编码  
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;

#endif // WIN32

	return result;
}

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValue& Value)
{
	switch (Value.Type)
	{
	case HazeValueType::Bool:
		return (HAZE_BINARY_CHAR*)&Value.Value.Bool;
	case HazeValueType::Short:
		return (HAZE_BINARY_CHAR*)&Value.Value.Short;
	case HazeValueType::UnsignedShort:
		return (HAZE_BINARY_CHAR*)&Value.Value.UnsignedShort;
	case HazeValueType::Int:
		return (HAZE_BINARY_CHAR*)&Value.Value.Int;
	case HazeValueType::Float:
		return (HAZE_BINARY_CHAR*)&Value.Value.Float;
	case HazeValueType::UnsignedInt:
		return (HAZE_BINARY_CHAR*)&Value.Value.UnsignedInt;
	case HazeValueType::Long:
		return (HAZE_BINARY_CHAR*)&Value.Value.Long;
	case HazeValueType::Double:
		return (HAZE_BINARY_CHAR*)&Value.Value.Double;
	case HazeValueType::UnsignedLong:
		return (HAZE_BINARY_CHAR*)&Value.Value.UnsignedLong;
	default:
		break;
	}
	return nullptr;
}
