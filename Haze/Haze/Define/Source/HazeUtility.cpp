#include "HazePch.h"
#include "HazeUtility.h"
#include "HazeLogDefine.h"
#include "HazeStrcut.h"
#include "HazeValue.h"
#include <chrono>

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
	case HazeToken::Int8:
	case HazeToken::UInt8:
	case HazeToken::Int16:
	case HazeToken::UInt16:
	case HazeToken::Int32:
	case HazeToken::UInt32:
	case HazeToken::Int64:
	case HazeToken::UInt64:
	case HazeToken::Float32:
	case HazeToken::Float64:
	case HazeToken::Enum:
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

HString NativeClassFunctionName(const HString& className, const HString& functionName)
{
	return functionName.substr((className + HAZE_CLASS_FUNCTION_CONBINE).length());
}

HString GetHazeModuleGlobalDataInitFunctionName(const HString& moduleName)
{
	return moduleName + HAZE_GLOBAL_DATA_INIT_FUNCTION;
}

const x_HChar* GetImportHeaderString()
{
	return HEADER_IMPORT_MODULE;
}

const x_HChar* GetImportHeaderModuleString()
{
	return HEADER_IMPORT_MODULE_MODULE;
}


const x_HChar* GetGlobalDataHeaderString()
{
	return HEADER_STRING_GLOBAL_DATA;
}

const x_HChar* GetStringTableHeaderString()
{
	return HEADER_STRING_STRING_TABLE;
}

const x_HChar* GetClassTableHeaderString()
{
	return HEADER_STRING_CLASS_TABLE;
}

const x_HChar* GetClassLabelHeader()
{
	return CLASS_LABEL_HEADER;
}

const x_HChar* GetFucntionTableHeaderString()
{
	return HEADER_STRING_FUNCTION_TABLE;
}

const x_HChar* GetClassFunctionLabelHeader()
{
	return CLASS_FUNCTION_LABEL_HEADER;
}

const x_HChar* GetFunctionLabelHeader()
{
	return FUNCTION_LABEL_HEADER;
}

const x_HChar* GetFunctionParamHeader()
{
	return FUNCTION_PARAM_HEADER;
}

const x_HChar* GetFunctionVariableHeader()
{
	return HAZE_LOCAL_VARIABLE_HEADER;
}

const x_HChar* GetFunctionTempRegisterHeader()
{
	return HAZE_LOCAL_TEMP_REGISTER_HEADER;
}

const x_HChar* GetFunctionStartHeader()
{
	return FUNCTION_START_HEADER;
}

const x_HChar* GetFunctionEndHeader()
{
	return FUNCTION_END_HEADER;
}

const x_HChar* GetClosureRefrenceVariableHeader()
{
	return CLOSURE_REF_VARIABLE;
}

const x_HChar* GetEnumTableLabelHeader()
{
	return HEADER_STRING_ENUM_TABLE;
}

const x_HChar* GetEnumStartHeader()
{
	return ENUM_START_HEADER;
}

const x_HChar* GetEnumEndHeader()
{
	return ENUM_END_HEADER;
}

const x_HChar* GetSymbolBeginHeader()
{
	return SYMBOL_BEGIN;
}

const x_HChar* GetSymbolEndHeader()
{
	return SYMBOL_END;
}

bool HazeIsSpace(x_HChar hChar, bool* isNewLine)
{
	if (isNewLine)
	{
		*isNewLine = hChar == x_HChar('\n');
	}

	return hChar == x_HChar(' ') || hChar == x_HChar('\n') || hChar == x_HChar('\t') || hChar == x_HChar('\v') || hChar == x_HChar('\f') || hChar == x_HChar('\r');
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
		return HazeValueType::Float32;
	}
	else
	{
		return HazeValueType::Int32;
	}

	return HazeValueType::Int32;
}

HString String2WString(const char* str)
{
	return String2WString(HAZE_BINARY_STRING(str));
}

HString String2WString(const HAZE_BINARY_STRING& str)
{
	HString& result = s_HazeString;
#ifdef _WIN32

	//��ȡ��������С
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0);
	result.resize(len);

	//���ֽڱ���ת���ɿ��ֽڱ���
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), result.data(), len);

#endif // WIN32

	return result;
}

HAZE_BINARY_STRING WString2String(const HString& wstr)
{
	HAZE_BINARY_STRING& result = s_String;

#ifdef WIN32

	//��ȡ��������С
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
	result.resize(len);

	//���ֽڱ���ת���ɶ��ֽڱ���
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
		pos = path.find(WindowsPathSlash, pos);
		if (pos != HString::npos)
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
	case HazeToken::StaticLibrary:
		return HazeLibraryType::Static;
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
	case HazeLibraryType::Static:
		return InstructionFunctionType::StaticLibFunction;
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
	case HazeValueType::Int8: \
		V = (V_TYPE)v2.Value.Int8; \
		break; \
	case HazeValueType::UInt8: \
		V = (V_TYPE)v2.Value.UInt8; \
		break; \
	case HazeValueType::Int16: \
		V = (V_TYPE)v2.Value.Int16; \
		break; \
	case HazeValueType::UInt16: \
		V = (V_TYPE)v2.Value.UInt16; \
		break; \
	case HazeValueType::Int32: \
		V = (V_TYPE)v2.Value.Int32; \
		break; \
	case HazeValueType::UInt32: \
		V = (V_TYPE)v2.Value.UInt32; \
		break; \
	case HazeValueType::Int64: \
		V = (V_TYPE)v2.Value.Int64; \
		break; \
	case HazeValueType::UInt64: \
		V = (V_TYPE)v2.Value.UInt64; \
		break; \
	case HazeValueType::Float32: \
		V = (V_TYPE)v2.Value.Float32; \
		break; \
	case HazeValueType::Float64: \
		V = (V_TYPE)v2.Value.Float64; \
		break; \
	default: \
		HAZE_LOG_ERR_W("��֧��<%s>����תΪ<%s>����", GetHazeValueTypeString(type2), GetHazeValueTypeString(type1)); \
		break; \
	} 

	switch (type1)
	{
	case HazeValueType::Int8:
	{
		CONVERT_SET_VALUE(v1.Value.Int8, x_int8)
	}
	break;
	case HazeValueType::UInt8:
	{
		CONVERT_SET_VALUE(v1.Value.UInt8, x_uint8)
	}
	break;
	case HazeValueType::Int16:
	{
		CONVERT_SET_VALUE(v1.Value.Int16, x_int16)
	}
	break;
	case HazeValueType::UInt16:
	{
		CONVERT_SET_VALUE(v1.Value.UInt16, x_uint16)
	}
	break;
	case HazeValueType::Int32:
	{
		CONVERT_SET_VALUE(v1.Value.Int32, x_int32)
	}
		break;
	case HazeValueType::UInt32:
	{
		CONVERT_SET_VALUE(v1.Value.UInt32, x_uint32)
	}
		break;
	case HazeValueType::Int64:
	{
		CONVERT_SET_VALUE(v1.Value.Int64, x_int64)
	}
		break;
	case HazeValueType::UInt64:
	{
		CONVERT_SET_VALUE(v1.Value.UInt64, x_int64)
	}
	break; 
	case HazeValueType::Float32:
	{
		CONVERT_SET_VALUE(v1.Value.Float32, x_float32)
	}
		break;
	case HazeValueType::Float64:
	{
		CONVERT_SET_VALUE(v1.Value.Float64, x_float64)
	}
		break;
	default:
		break;
	}
}

V_Array<HString> HazeStringSplit(const HString& str, const HString& delimiter)
{
	V_Array<HString> result;

	x_HChar* s = new x_HChar[str.size() + 1];
	s[str.size()] = '\0';
	
	wcscpy_s(s, str.size() + 1, str.c_str());

	x_HChar* p = nullptr;
	auto token = wcstok_s(s, delimiter.c_str(), &p);
	while (token)
	{
		result.push_back(token);
		token = wcstok_s(NULL, delimiter.c_str(), &p);
	}

	return result;
}