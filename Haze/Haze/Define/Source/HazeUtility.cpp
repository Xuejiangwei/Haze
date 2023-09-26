#include "HazeUtility.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <string>
#include <regex>

thread_local static HAZE_STRING s_HazeString;
thread_local static HAZE_BINARY_STRING s_String;

bool IsAndOrToken(HazeToken token)
{
	return token == HazeToken::And || token == HazeToken::Or;
}

bool IsAndToken(HazeToken token)
{
	return token == HazeToken::And;
}

bool IsOrToken(HazeToken token)
{
	return token == HazeToken::Or;
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

const HAZE_CHAR* GetFunctionVariableHeader()
{
	return HAZE_LOCAL_VARIABLE_HEADER;
}

const HAZE_CHAR* GetFunctionStartHeader()
{
	return FUNCTION_START_HEADER;
}

const HAZE_CHAR* GetFunctionEndHeader()
{
	return FUNCTION_END_HEADER;
}

bool HazeIsSpace(HAZE_CHAR hChar, bool* isNewLine)
{
	if (isNewLine)
	{
		*isNewLine = hChar == HAZE_CHAR('\n');
	}

	return hChar == HAZE_CHAR(' ') || hChar == HAZE_CHAR('\n') || hChar == HAZE_CHAR('\t') || hChar == HAZE_CHAR('\v') || hChar == HAZE_CHAR('\f') || hChar == HAZE_CHAR('\r');
}

bool IsNumber(const HAZE_STRING& str)
{
	std::wregex pattern(HAZE_TEXT("-[0-9]+(.[0-9]+)?|[0-9]+(.[0-9]+)?"));
	return std::regex_match(str, pattern);
}

HazeValueType GetNumberDefaultType(const HAZE_STRING& str)
{
	std::wregex pattern(HAZE_TEXT("-?(([1-9]\\d*\\.\\d*)|(0\\.\\d*[1-9]\\d*))"));
	bool isFloat = std::regex_match(str, pattern);
	
	if (isFloat)
	{
		return HazeValueType::Float;
	}
	else
	{
		return HazeValueType::Int;
	}

	return HazeValueType::Int;
}

HAZE_STRING String2WString(const char* str)
{
	return String2WString(HAZE_BINARY_STRING(str));
}

HAZE_STRING String2WString(const HAZE_BINARY_STRING& str)
{
	HAZE_STRING& result = s_HazeString;
#ifdef _WIN32

	//获取缓冲区大小
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0);
	result.resize(len);

	//多字节编码转换成宽字节编码
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), result.data(), len);

#endif // WIN32

	return result;
}

HAZE_BINARY_STRING WString2String(const HAZE_STRING& wstr)
{
	HAZE_BINARY_STRING& result = s_String;

#ifdef WIN32

	//获取缓冲区大小
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
	result.resize(len);

	//宽字节编码转换成多字节编码
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), result.data(), len, NULL, NULL);

#endif // WIN32

	return result;
}

char* UTF8_2_GB2312(const char* utf8)
{
	HAZE_STRING& hazeString = s_HazeString;
	HAZE_BINARY_STRING& result = s_String;

#ifdef _WIN32

	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	hazeString.resize(len);
	
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, hazeString.data(), len);

	len = WideCharToMultiByte(CP_ACP, 0, hazeString.data(), -1, NULL, 0, NULL, NULL);
	result.resize(len);

	WideCharToMultiByte(CP_ACP, 0, hazeString.data(), -1, result.data(), len, NULL, NULL);

#endif

	return result.data();

}

char* GB2312_2_UFT8(const char* gb2312)
{
	HAZE_STRING& hazeString = s_HazeString;
	HAZE_BINARY_STRING& result = s_String;

#ifdef _WIN32

	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	hazeString.resize(len);

	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, hazeString.data(), len);

	len = WideCharToMultiByte(CP_UTF8, 0, hazeString.data(), -1, NULL, 0, NULL, NULL);
	result.resize(len);

	WideCharToMultiByte(CP_UTF8, 0, hazeString.data(), -1, result.data(), len, NULL, NULL);

#endif

	return result.data();
}

void ReplacePathSlash(HAZE_STRING& Path)
{
	static HAZE_STRING WindowsPathSlash(HAZE_TEXT("\\"));
	static HAZE_STRING PathSlash(HAZE_TEXT("/"));

	for (HAZE_STRING::size_type pos(0); pos != HAZE_STRING::npos; pos += PathSlash.length())
	{
		if ((pos = Path.find(WindowsPathSlash, pos)) != HAZE_STRING::npos)
		{
			Path.replace(pos, WindowsPathSlash.length(), PathSlash);
		}
		else
		{
			break;
		}
	}
}

HazeLibraryType GetHazeLibraryTypeByToken(HazeToken m_Token)
{
	switch (m_Token)
	{
	case HazeToken::StandardLibrary:
		return HazeLibraryType::Standard;
		break;
	case HazeToken::DLLLibrary:
		return HazeLibraryType::DLL;
		break;
	default:
		return HazeLibraryType::Normal;
		break;
	}
}

InstructionFunctionType GetFunctionTypeByLibraryType(HazeLibraryType m_Type)
{
	switch (m_Type)
	{
	case HazeLibraryType::Normal:
		return InstructionFunctionType::HazeFunction;
	case HazeLibraryType::Standard:
		return InstructionFunctionType::StdLibFunction;
	case HazeLibraryType::DLL:
		return InstructionFunctionType::DLLLibFunction;
	default:
		return InstructionFunctionType::HazeFunction;
	}
}

HAZE_STRING GetModuleNameByFilePath(const HAZE_STRING& FilePath)
{
	auto Index = FilePath.find_last_of(HAZE_TEXT("\\"));
	if (Index != HAZE_STRING::npos)
	{
		return FilePath.substr(Index + 1, FilePath.length() - Index - 1 - 3);
	}

	Index = FilePath.find_last_of(HAZE_TEXT("/"));
	if (Index != HAZE_STRING::npos)
	{
		return FilePath.substr(Index + 1, FilePath.length() - Index - 1 - 3);
	}

	return HAZE_TEXT("None");
}

HAZE_BINARY_STRING ToString(void* Value)
{
	std::stringstream SS;
	SS << Value;
	return SS.str();
}