#include "HazeUtility.h"

#include <Windows.h>
#include <string>
#include <regex>

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

bool HazeIsSpace(HAZE_CHAR Char)
{
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

HAZE_STRING String2WString(const HAZE_BINARY_STRING& str)
{
	HAZE_STRING result;
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


HAZE_BINARY_STRING WString2String(const HAZE_STRING& wstr)
{
	HAZE_BINARY_STRING result;

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