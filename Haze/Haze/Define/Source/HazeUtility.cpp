#include "HazePch.h"
#include "HazeUtility.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <regex>

thread_local static HString s_HazeString;
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

bool IsCanCastToken(HazeToken token)
{
	switch (token)
	{
	case HazeToken::Bool:
	case HazeToken::Byte:
	case HazeToken::Char:
	case HazeToken::Short:
	case HazeToken::Int:
	case HazeToken::Float:
	case HazeToken::Long:
	case HazeToken::Double:
	case HazeToken::UnsignedByte:
	case HazeToken::UnsignedShort:
	case HazeToken::UnsignedInt:
	case HazeToken::UnsignedLong:
	case HazeToken::Enum:
	case HazeToken::ReferenceBase:
	case HazeToken::ReferenceClass:
	case HazeToken::PointerBase:
	case HazeToken::PointerClass:
	case HazeToken::PointerFunction:
	case HazeToken::PointerPointer:
		return true;
	default:
		break;
	}
	return false;
}

int Log2(int n)
{
	int count = 0;
	if (n == 1)
	{
		return 0;
	}

	while (n > 1)
	{
		n = n >> 1;
		count++;
	}

	return count;
}

HString GetHazeClassFunctionName(const HString& className, const HString& functionName)
{
	return className + HAZE_CLASS_FUNCTION_CONBINE + functionName;
}

const HChar* GetGlobalDataHeaderString()
{
	return HEADER_STRING_GLOBAL_DATA;
}

const HChar* GetGlobalDataInitBlockStart()
{
	return GLOBAL_DATA_INIT_BLOCK_START;
}

const HChar* GetGlobalDataInitBlockEnd()
{
	return GLOBAL_DATA_INIT_BLOCK_END;
}

const HChar* GetStringTableHeaderString()
{
	return HEADER_STRING_STRING_TABLE;
}

const HChar* GetClassTableHeaderString()
{
	return HEADER_STRING_CLASS_TABLE;
}

const HChar* GetClassLabelHeader()
{
	return CLASS_LABEL_HEADER;
}

const HChar* GetFucntionTableHeaderString()
{
	return HEADER_STRING_FUNCTION_TABLE;
}

const HChar* GetFunctionLabelHeader()
{
	return FUNCTION_LABEL_HEADER;
}

const HChar* GetFunctionParamHeader()
{
	return FUNCTION_PARAM_HEADER;
}

const HChar* GetFunctionVariableHeader()
{
	return HAZE_LOCAL_VARIABLE_HEADER;
}

const HChar* GetFunctionStartHeader()
{
	return FUNCTION_START_HEADER;
}

const HChar* GetFunctionEndHeader()
{
	return FUNCTION_END_HEADER;
}

bool HazeIsSpace(HChar hChar, bool* isNewLine)
{
	if (isNewLine)
	{
		*isNewLine = hChar == HChar('\n');
	}

	return hChar == HChar(' ') || hChar == HChar('\n') || hChar == HChar('\t') || hChar == HChar('\v') || hChar == HChar('\f') || hChar == HChar('\r');
}

bool IsNumber(const HString& str)
{
	std::wregex pattern(H_TEXT("-[0-9]+(.[0-9]+)?|[0-9]+(.[0-9]+)?"));
	return std::regex_match(str, pattern);
}

HazeValueType GetNumberDefaultType(const HString& str)
{
	std::wregex pattern(H_TEXT("-?(([1-9]\\d*\\.\\d*)|(0\\.\\d*[1-9]\\d*))"));
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

HString String2WString(const char* str)
{
	return String2WString(HAZE_BINARY_STRING(str));
}

HString String2WString(const HAZE_BINARY_STRING& str)
{
	HString& result = s_HazeString;
#ifdef _WIN32

	//获取缓冲区大小
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0);
	result.resize(len);

	//多字节编码转换成宽字节编码
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), result.data(), len);

#endif // WIN32

	return result;
}

HAZE_BINARY_STRING WString2String(const HString& wstr)
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
	HString& hazeString = s_HazeString;
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
	HString& hazeString = s_HazeString;
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

void ReplacePathSlash(HString& path)
{
	static HString WindowsPathSlash(H_TEXT("\\"));
	static HString PathSlash(H_TEXT("/"));

	for (HString::size_type pos(0); pos != HString::npos; pos += PathSlash.length())
	{
		if ((pos = path.find(WindowsPathSlash, pos)) != HString::npos)
		{
			path.replace(pos, WindowsPathSlash.length(), PathSlash);
		}
		else
		{
			break;
		}
	}
}

HazeLibraryType GetHazeLibraryTypeByToken(HazeToken token)
{
	switch (token)
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

InstructionFunctionType GetFunctionTypeByLibraryType(HazeLibraryType type)
{
	switch (type)
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

HString GetModuleNameByFilePath(const HString& filePath)
{
	auto Index = filePath.find_last_of(H_TEXT("\\"));
	if (Index != HString::npos)
	{
		return filePath.substr(Index + 1, filePath.length() - Index - 1 - 3);
	}

	Index = filePath.find_last_of(H_TEXT("/"));
	if (Index != HString::npos)
	{
		return filePath.substr(Index + 1, filePath.length() - Index - 1 - 3);
	}

	return H_TEXT("None");
}

HAZE_BINARY_STRING ToString(void* value)
{
	std::stringstream ss;
	ss << value;

	return ss.str();
}


void ConvertBaseTypeValue(HazeValueType type1, HazeValue& v1, HazeValueType type2, const HazeValue& v2)
{
#define CONVERT_SET_VALUE(V, V_TYPE) switch(type2) { \
	case HazeValueType::Byte: \
		V = (V_TYPE)v2.Value.Byte; \
		break; \
	case HazeValueType::UnsignedByte: \
		V = (V_TYPE)v2.Value.UnsignedByte; \
		break; \
	case HazeValueType::Char: \
		V = (V_TYPE)v2.Value.Char; \
		break; \
	case HazeValueType::Short: \
		V = (V_TYPE)v2.Value.Short; \
		break; \
	case HazeValueType::UnsignedShort: \
		V = (V_TYPE)v2.Value.UnsignedShort; \
		break; \
	case HazeValueType::Int: \
		V = (V_TYPE)v2.Value.Int; \
		break; \
	case HazeValueType::UnsignedInt: \
		V = (V_TYPE)v2.Value.UnsignedInt; \
		break; \
	case HazeValueType::Float: \
		V = (V_TYPE)v2.Value.Float; \
		break; \
	case HazeValueType::Long: \
		V = (V_TYPE)v2.Value.Long; \
		break; \
	case HazeValueType::UnsignedLong: \
		V = (V_TYPE)v2.Value.UnsignedLong; \
		break; \
	case HazeValueType::Double: \
		V = (V_TYPE)v2.Value.Double; \
		break; \
	default: \
		HAZE_LOG_ERR_W("不支持<%s>类型转为<%s>类型", GetHazeValueTypeString(type2), GetHazeValueTypeString(type1)); \
		break; \
	} 

	switch (type1)
	{
	case HazeValueType::Byte:
	{
		CONVERT_SET_VALUE(v1.Value.Byte, HByte)
	}
	break;
	case HazeValueType::UnsignedByte:
	{
		CONVERT_SET_VALUE(v1.Value.UnsignedByte, uhbyte)
	}
	break;
	case HazeValueType::Char:
	{
		CONVERT_SET_VALUE(v1.Value.Char, HChar)
	}
		break;
	case HazeValueType::Short:
	{
		CONVERT_SET_VALUE(v1.Value.Short, short)
	}
	break;
	case HazeValueType::UnsignedShort:
	{
		CONVERT_SET_VALUE(v1.Value.UnsignedShort, ushort)
	}
	break;
	case HazeValueType::Int:
	{
		CONVERT_SET_VALUE(v1.Value.Int, int)
	}
		break;
	case HazeValueType::UnsignedInt:
	{
		CONVERT_SET_VALUE(v1.Value.Int, int)
	}
		break;
	case HazeValueType::Float:
	{
		CONVERT_SET_VALUE(v1.Value.Float, float)
	}
		break;
	case HazeValueType::Long:
	{
		CONVERT_SET_VALUE(v1.Value.Long, int64)
	}
		break;
	case HazeValueType::UnsignedLong:
	{
		CONVERT_SET_VALUE(v1.Value.UnsignedLong, int64)
	}
	break; 
	case HazeValueType::Double:
	{
		CONVERT_SET_VALUE(v1.Value.Double, double)
	}
		break;
	default:
		break;
	}
}

V_Array<HString> HazeStringSplit(const HString& str, const HString& delimiter)
{
	V_Array<HString> result;

	HChar* s = new HChar[str.size() + 1];
	s[str.size()] = '\0';
	
	wcscpy_s(s, str.size() + 1, str.c_str());

	HChar* p = nullptr;
	auto token = wcstok_s(s, delimiter.c_str(), &p);
	while (token)
	{
		result.push_back(token);
		token = wcstok_s(NULL, delimiter.c_str(), &p);
	}

	return result;
}
