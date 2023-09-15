#include "HazeUtility.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <string>
#include <regex>

bool IsAndOrToken(HazeToken Token)
{
	return Token == HazeToken::And || Token == HazeToken::Or;
}

bool IsAndToken(HazeToken Token)
{
	return Token == HazeToken::And;
}

bool IsOrToken(HazeToken Token)
{
	return Token == HazeToken::Or;
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

bool HazeIsSpace(HAZE_CHAR Char, bool* IsNewLine)
{
	if (IsNewLine)
	{
		*IsNewLine = Char == HAZE_CHAR('\n');
	}

	return Char == HAZE_CHAR(' ') || Char == HAZE_CHAR('\n') || Char == HAZE_CHAR('\t') || Char == HAZE_CHAR('\v') || Char == HAZE_CHAR('\f') || Char == HAZE_CHAR('\r');
}

bool IsNumber(const HAZE_STRING& Str)
{
	std::wregex Pattern(HAZE_TEXT("-[0-9]+(.[0-9]+)?|[0-9]+(.[0-9]+)?"));
	return std::regex_match(Str, Pattern);
}

HazeValueType GetNumberDefaultType(const HAZE_STRING& Str)
{
	std::wregex Pattern(HAZE_TEXT("-?(([1-9]\\d*\\.\\d*)|(0\\.\\d*[1-9]\\d*))"));
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

HAZE_STRING String2WString(const char* Str)
{
	return String2WString(HAZE_BINARY_STRING(Str));
}

HAZE_STRING String2WString(const HAZE_BINARY_STRING& str)
{
	HAZE_STRING result;
#ifdef _WIN32

	//获取缓冲区大小，并申请空间，缓冲区大小按字符计算
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	//多字节编码转换成宽字节编码
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), buffer, len);
	buffer[len] = '\0';             //添加字符串结尾
	//删除缓冲区并返回值
	result.append(buffer);
	delete[] buffer;

#endif // WIN32

	return result;
}

HAZE_BINARY_STRING WString2String(const HAZE_STRING& wstr)
{
	HAZE_BINARY_STRING result;

#ifdef WIN32

	//获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
	char* buffer = new char[len + 1];
	//宽字节编码转换成多字节编码
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	//删除缓冲区并返回值
	result.append(buffer);
	delete[] buffer;

#endif // WIN32

	return result;
}

char* UTF8_2_GB2312(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

char* GB2312_2_UFT8(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
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

HazeLibraryType GetHazeLibraryTypeByToken(HazeToken Token)
{
	switch (Token)
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

InstructionFunctionType GetFunctionTypeByLibraryType(HazeLibraryType Type)
{
	switch (Type)
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