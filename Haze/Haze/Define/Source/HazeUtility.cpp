#include "HazePch.h"
#include "HazeUtility.h"
#include "HazeLogDefine.h"
#include "HazeStrcut.h"
#include <chrono>

#ifdef _WIN32
	#include <windows.h>
#endif

#include <fstream>
#include <locale>
#include <codecvt>
#include "HazeCompilerVersion.h"  // 添加版本检测头文件

#include <regex>

thread_local static STDString s_HazeString;
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
	case HazeToken::CustomEnum:
	case HazeToken::CustomClass:
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

STDString GetHazeClassFunctionName(const STDString& className, const STDString& functionName)
{
	return className + HAZE_CLASS_FUNCTION_CONBINE + functionName;
}

STDString NativeClassFunctionName(const STDString& className, const STDString& functionName)
{
	return functionName.substr((className + HAZE_CLASS_FUNCTION_CONBINE).length());
}

STDString GetHazeModuleGlobalDataInitFunctionName(const STDString& moduleName)
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

const x_HChar* GetRefTypeIdString()
{
	return HEADER_REF_TYPE_ID;
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

const x_HChar* GetBlockFlowHeader()
{
	return BLOCK_FLOW_HEADER;
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

//const x_HChar* GetSymbolBeginHeader()
//{
//	return SYMBOL_BEGIN;
//}
//
//const x_HChar* GetSymbolEndHeader()
//{
//	return SYMBOL_END;
//}

const x_HChar* GetTypeInfoFunctionBeginHeader()
{
	return TYPE_INFO_FUNC_PARAM_BEGIN;
}

const x_HChar* GetTypeInfoFunctionEndHeader()
{
	return TYPE_INFO_FUNC_PARAM_END;
}

const x_HChar* GetTypeInfoBeginHeader()
{
	return TYPE_INFO_BEGIN;
}

const x_HChar* GetTypeInfoEndHeader()
{
	return TYPE_INFO_END;
}

bool HazeIsSpace(x_HChar hChar, bool* isNewLine)
{
	if (isNewLine)
	{
		*isNewLine = hChar == x_HChar('\n');
	}

	return hChar == x_HChar(' ') || hChar == x_HChar('\n') || hChar == x_HChar('\t') || hChar == x_HChar('\v') || hChar == x_HChar('\f') || hChar == x_HChar('\r');
}

bool IsNumber(const STDString& str)
{
	std::wregex pattern(H_TEXT("-[0-9]+(.[0-9]+)?|[0-9]+(.[0-9]+)?"));
	return std::regex_match(str, pattern);
}

HazeValueType GetNumberDefaultType(const STDString& str)
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

STDString String2WString(const char* str)
{
	return String2WString(HAZE_BINARY_STRING(str));
}

STDString String2WString(const HAZE_BINARY_STRING& str)
{
	STDString& result = s_HazeString;
#ifdef _WIN32

	//��ȡ��������С
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0);
	result.resize(len);

	//���ֽڱ���ת���ɿ��ֽڱ���
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), result.data(), len);

#endif // WIN32

	return result;
}

HAZE_BINARY_STRING WString2String(const STDString& wstr)
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

bool IsUtf8Bom(const char* utf8)
{
	return (x_uint8)utf8[0] == 0xEF && (x_uint8)utf8[1] == 0xBB && (x_uint8)utf8[2] == 0xBF;
}

STDString ReadUtf8File(const STDString& filePath)
{
	std::wstring content;
	bool success = false;

	// 方法1：尝试使用std::codecvt_utf8（C++11标准方法）
#if HAZE_HAS_CODECVT
	try {
		std::ifstream fs(filePath, std::ios::binary);
		if (fs.is_open()) {
			std::string utf8_content((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
			fs.close();

			if (!utf8_content.empty()) {
				// 处理UTF-8 BOM
				const char* utf8_data = utf8_content.c_str();
				size_t utf8_size = utf8_content.size();

				if (utf8_size >= 3 &&
					(unsigned char)utf8_data[0] == 0xEF &&
					(unsigned char)utf8_data[1] == 0xBB &&
					(unsigned char)utf8_data[2] == 0xBF) {
					utf8_data += 3;
					utf8_size -= 3;
				}

				// 使用codecvt转换
				std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
				content = converter.from_bytes(utf8_data, utf8_data + utf8_size);
				success = true;

				// 调试信息
				HAZE_LOG_INFO_W("使用std::codecvt_utf8读取UTF-8文件 [%s %s]: %s\n",
					HAZE_COMPILER_STRING, HAZE_CPP_VERSION_STRING, filePath.c_str());
			}
		}
	}
	catch (const std::exception&) {
		success = false;
	}
#endif

	// 方法2：在Windows上尝试使用UTF-8 locale（Windows 10 1903+）
#if HAZE_HAS_WIN32_UTF8_LOCALE
	if (!success) {
		try {
			std::locale utf8_locale(".utf8");
			std::wifstream fs(filePath);
			fs.imbue(utf8_locale);

			if (fs.is_open())
			{
				content = std::wstring((std::istreambuf_iterator<wchar_t>(fs)), std::istreambuf_iterator<wchar_t>());
				fs.close();

				if (!content.empty())
				{
					// 处理可能的BOM
					if (content.size() >= 1 && content[0] == 0xFEFF) {
						content.erase(0, 1);
					}
					success = true;

					// 调试信息
					HAZE_LOG_INFO_W("使用Windows UTF-8 locale读取文件 [%s]: %s\n",
						H_TEXT(HAZE_COMPILER_STRING), filePath.c_str());
				}
			}
		}
		catch (const std::exception&) {
			success = false;
		}
	}
#endif

	// 方法3：备选方案 - 使用手动转换（确保兼容性）
	if (!success) {
		std::ifstream binaryFs(filePath, std::ios::binary);
		if (binaryFs.is_open()) {
			std::string utf8Content((std::istreambuf_iterator<char>(binaryFs)), std::istreambuf_iterator<char>());
			binaryFs.close();

			if (!utf8Content.empty()) {
				// 处理UTF-8 BOM
				const char* utf8Data = utf8Content.c_str();
				size_t utf8Size = utf8Content.size();

				if (utf8Size >= 3 && IsUtf8Bom(utf8Data))
				{
					utf8Data += 3;
					utf8Size -= 3;
				}

#if HAZE_PLATFORM_WINDOWS
				// Windows: 使用MultiByteToWideChar
				if (utf8Size > 0) {
					int len = MultiByteToWideChar(CP_UTF8, 0, utf8Data, (int)utf8Size, NULL, 0);
					if (len > 0) {
						content.resize(len);
						MultiByteToWideChar(CP_UTF8, 0, utf8Data, (int)utf8Size, content.data(), len);
						success = true;

						// 调试信息
						HAZE_LOG_INFO_W("使用MultiByteToWideChar读取UTF-8文件 [%s]: %s\n",
							H_TEXT(HAZE_COMPILER_STRING), filePath.c_str());
					}
				}
#else
				// Linux/Mac: 使用简单转换
				if (utf8Size > 0) {
					content.reserve(utf8Size);
					for (size_t i = 0; i < utf8Size; ++i) {
						content.push_back(static_cast<wchar_t>(static_cast<unsigned char>(utf8Data[i])));
					}
					success = true;

					// 调试信息
					HAZE_LOG_INFO_W("使用简单字符转换读取UTF-8文件 [%s]: %s\n",
						HAZE_COMPILER_STRING, filePath.c_str());
				}
#endif
			}
		}
	}

	if (!success) {
		HAZE_LOG_ERR_W("无法读取文件 [%s %s]: %s\n", H_TEXT(HAZE_COMPILER_STRING), H_TEXT(HAZE_CPP_VERSION_STRING), filePath.c_str());
	}

	return content;
}

char* UTF8_2_GB2312(const char* utf8)
{
	STDString& hazeString = s_HazeString;
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
	STDString& hazeString = s_HazeString;
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

void ReplacePathSlash(STDString& path)
{
	static STDString WindowsPathSlash(H_TEXT("\\"));
	static STDString PathSlash(H_TEXT("/"));

	for (STDString::size_type pos(0); pos != STDString::npos; pos += PathSlash.length())
	{
		pos = path.find(WindowsPathSlash, pos);
		if (pos != STDString::npos)
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

STDString GetModuleNameByFilePath(const STDString& filePath)
{
	auto Index = filePath.find_last_of(H_TEXT("\\"));
	if (Index != STDString::npos)
	{
		return filePath.substr(Index + 1, filePath.length() - Index - 1 - 3);
	}

	Index = filePath.find_last_of(H_TEXT("/"));
	if (Index != STDString::npos)
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
	case HazeValueType::Enum: \
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

V_Array<STDString> HazeStringSplit(const STDString& str, const STDString& delimiter)
{
	V_Array<STDString> result;

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

HazeValueType GetHazeBaseTypeByDesc(HazeDataDesc desc)
{
	switch (desc)
	{
		case HazeDataDesc::ConstantBool:
			return HazeValueType::Bool;
		case HazeDataDesc::ConstantInt8:
			return HazeValueType::Int8;
		case HazeDataDesc::ConstantUInt8:
			return HazeValueType::UInt8;
		case HazeDataDesc::ConstantInt16:
			return HazeValueType::Int16;
		case HazeDataDesc::ConstantUInt16:
			return HazeValueType::UInt16;
		case HazeDataDesc::ConstantInt32:
			return HazeValueType::Int32;
		case HazeDataDesc::ConstantUInt32:
			return HazeValueType::UInt32;
		case HazeDataDesc::ConstantInt64:
			return HazeValueType::Int64;
		case HazeDataDesc::ConstantUInt64:
			return HazeValueType::UInt64;
		case HazeDataDesc::ConstantFloat32:
			return HazeValueType::Float32;
		case HazeDataDesc::ConstantFloat64:
			return HazeValueType::Float64;
		default:
			break;
	}

	HAZE_LOG_ERR_W("未获得正确的类型!\n");
	return HazeValueType::None;
}

#include "HazeCompilerVersion.h"

// 显示编译环境信息
void ShowCompilerInfo()
{
	HAZE_LOG_INFO_W("=== Haze编译环境信息 ===\n");
	
	// 测试宏展开（可选的调试代码）
	const char* cpp_version = HAZE_CPP_VERSION_STRING;
	HAZE_LOG_INFO_W("C++标准版本: %s\n", cpp_version);
	
	HAZE_LOG_INFO_W("编译器: %s\n", HAZE_COMPILER_STRING);
	
	HAZE_LOG_INFO_W("支持的特性:\n");
	HAZE_LOG_INFO_W("  - std::codecvt: %s\n", HAZE_HAS_CODECVT ? "✓" : "✗");
	HAZE_LOG_INFO_W("  - std::filesystem: %s\n", HAZE_HAS_FILESYSTEM ? "✓" : "✗");
	HAZE_LOG_INFO_W("  - std::string_view: %s\n", HAZE_HAS_STRING_VIEW ? "✓" : "✗");
	HAZE_LOG_INFO_W("  - Windows UTF-8 locale: %s\n", HAZE_HAS_WIN32_UTF8_LOCALE ? "✓" : "✗");
	
	HAZE_LOG_INFO_W("目标平台: ");
#if HAZE_PLATFORM_WINDOWS
	HAZE_LOG_INFO_W("Windows\n");
#elif HAZE_PLATFORM_LINUX
	HAZE_LOG_INFO_W("Linux\n");
#elif HAZE_PLATFORM_MAC
	HAZE_LOG_INFO_W("macOS\n");
#else
	HAZE_LOG_INFO_W("Unknown\n");
#endif

	HAZE_LOG_INFO_W("编译器版本详情:\n");
#if HAZE_COMPILER_MSVC
	HAZE_LOG_INFO_W("  - MSVC版本号: %d\n", _MSC_VER);
	HAZE_LOG_INFO_W("  - Visual Studio: %d\n", HAZE_MSVC_VERSION);
#elif HAZE_COMPILER_GCC
	HAZE_LOG_INFO_W("  - GCC版本: %d.%d\n", __GNUC__, __GNUC_MINOR__);
#elif HAZE_COMPILER_CLANG
	HAZE_LOG_INFO_W("  - Clang版本: %d.%d\n", __clang_major__, __clang_minor__);
#endif

	HAZE_LOG_INFO_W("原始__cplusplus值: %ld\n", __cplusplus);
	HAZE_LOG_INFO_W("========================\n");
}