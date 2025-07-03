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

// 类型注册表实现
x_uint32 HazeTypeRegistry::RegisterType(const HazeDefineType& type, const HString& name, const HString& module)
{
	TemplateDefineTypes emptyTemplates;
	return RegisterComplexType(type, name, module, emptyTemplates, 0);
}

x_uint32 HazeTypeRegistry::RegisterComplexType(const HazeDefineType& type, const HString& name, const HString& module,
	const TemplateDefineTypes& templateTypes, x_uint64 arrayDimension)
{
	// 生成唯一类型名称
	HString uniqueName = GenerateUniqueTypeName(type, name, module, templateTypes, arrayDimension);
	
	// 检查是否已经注册
	auto it = m_TypeNameToId.find(uniqueName);
	if (it != m_TypeNameToId.end())
	{
		return it->second;
	}
	
	// 创建新的类型信息
	auto typeInfo = MakeShare<HazeTypeInfo>(type, name, module);
	typeInfo->TypeId = m_NextTypeId++;
	typeInfo->Size = GetSizeByHazeType(type.PrimaryType);
	typeInfo->IsGeneric = templateTypes.Types.size() > 0;
	typeInfo->IsArray = IsArrayType(type.PrimaryType);
	typeInfo->ArrayDimension = arrayDimension;
	typeInfo->CreateTime = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
	
	// 处理泛型参数
	if (typeInfo->IsGeneric)
	{
		for (const auto& templateType : templateTypes.Types)
		{
			if (!templateType.IsDefines && templateType.Type)
			{
				// 递归注册泛型参数类型
				x_uint32 paramTypeId = RegisterType(templateType.Type->BaseType, 
					templateType.Type->BaseType.GetFullTypeName(), module);
				auto paramTypeInfo = GetTypeInfo(paramTypeId);
				if (paramTypeInfo)
				{
					typeInfo->GenericParams.push_back(paramTypeInfo);
				}
			}
		}
	}
	
	// 注册类型
	m_TypeInfos[typeInfo->TypeId] = typeInfo;
	m_TypeNameToId[uniqueName] = typeInfo->TypeId;
	
	HAZE_LOG_INFO(H_TEXT("注册类型: %s (ID: %d, Module: %s)\n"), 
		uniqueName.c_str(), typeInfo->TypeId, module.c_str());
	
	return typeInfo->TypeId;
}

Share<HazeTypeInfo> HazeTypeRegistry::GetTypeInfo(x_uint32 typeId)
{
	auto it = m_TypeInfos.find(typeId);
	if (it != m_TypeInfos.end())
	{
		return it->second;
	}
	return nullptr;
}

Share<HazeTypeInfo> HazeTypeRegistry::GetTypeInfo(const HString& typeName, const HString& module)
{
	HString searchName = module.empty() ? typeName : module + H_TEXT("::") + typeName;
	auto it = m_TypeNameToId.find(searchName);
	if (it != m_TypeNameToId.end())
	{
		return GetTypeInfo(it->second);
	}
	return nullptr;
}

bool HazeTypeRegistry::IsTypeRegistered(const HazeDefineType& type, const HString& name, const HString& module)
{
	TemplateDefineTypes emptyTemplates;
	HString uniqueName = GenerateUniqueTypeName(type, name, module, emptyTemplates, 0);
	return m_TypeNameToId.find(uniqueName) != m_TypeNameToId.end();
}

HString HazeTypeRegistry::GetTypeDescription(x_uint32 typeId)
{
	auto typeInfo = GetTypeInfo(typeId);
	if (!typeInfo)
	{
		return H_TEXT("Unknown Type");
	}
	
	HAZE_STRING_STREAM hss;
	hss << H_TEXT("Type: ") << typeInfo->TypeName;
	hss << H_TEXT(" (ID: ") << typeInfo->TypeId << H_TEXT(")");
	hss << H_TEXT(" Module: ") << typeInfo->ModuleName;
	hss << H_TEXT(" Size: ") << typeInfo->Size;
	
	if (typeInfo->IsArray)
	{
		hss << H_TEXT(" Array[") << typeInfo->ArrayDimension << H_TEXT("]");
	}
	
	if (typeInfo->IsGeneric)
	{
		hss << H_TEXT(" Generic<");
		for (size_t i = 0; i < typeInfo->GenericParams.size(); ++i)
		{
			if (i > 0) hss << H_TEXT(", ");
			hss << typeInfo->GenericParams[i]->TypeName;
		}
		hss << H_TEXT(">");
	}
	
	return hss.str();
}

bool HazeTypeRegistry::IsTypeCompatible(x_uint32 typeId1, x_uint32 typeId2)
{
	auto type1 = GetTypeInfo(typeId1);
	auto type2 = GetTypeInfo(typeId2);
	
	if (!type1 || !type2)
	{
		return false;
	}
	
	// 相同类型
	if (typeId1 == typeId2)
	{
		return true;
	}
	
	// 基础类型兼容性检查
	if (type1->BaseType.PrimaryType == type2->BaseType.PrimaryType)
	{
		return true;
	}
	
	// 数值类型的兼容性
	if (IsNumberType(type1->BaseType.PrimaryType) && IsNumberType(type2->BaseType.PrimaryType))
	{
		return GetStrongerType(type1->BaseType.PrimaryType, type2->BaseType.PrimaryType, false) != HazeValueType::None;
	}
	
	return false;
}

void HazeTypeRegistry::SerializeTypeInfo(HAZE_STRING_STREAM& hss, x_uint32 typeId)
{
	auto typeInfo = GetTypeInfo(typeId);
	if (!typeInfo)
	{
		return;
	}
	
	hss << H_TEXT("TypeInfo ") << typeInfo->TypeId << H_TEXT(" {") << HAZE_ENDL;
	hss << H_TEXT("  Name: ") << typeInfo->TypeName << HAZE_ENDL;
	hss << H_TEXT("  Module: ") << typeInfo->ModuleName << HAZE_ENDL;
	hss << H_TEXT("  Size: ") << typeInfo->Size << HAZE_ENDL;
	hss << H_TEXT("  IsGeneric: ") << (typeInfo->IsGeneric ? H_TEXT("true") : H_TEXT("false")) << HAZE_ENDL;
	hss << H_TEXT("  IsArray: ") << (typeInfo->IsArray ? H_TEXT("true") : H_TEXT("false")) << HAZE_ENDL;
	hss << H_TEXT("  ArrayDimension: ") << typeInfo->ArrayDimension << HAZE_ENDL;
	hss << H_TEXT("  CreateTime: ") << typeInfo->CreateTime << HAZE_ENDL;
	
	if (typeInfo->IsGeneric && !typeInfo->GenericParams.empty())
	{
		hss << H_TEXT("  GenericParams: [");
		for (size_t i = 0; i < typeInfo->GenericParams.size(); ++i)
		{
			if (i > 0) hss << H_TEXT(", ");
			hss << typeInfo->GenericParams[i]->TypeId;
		}
		hss << H_TEXT("]") << HAZE_ENDL;
	}
	
	hss << H_TEXT("}") << HAZE_ENDL;
}

x_uint32 HazeTypeRegistry::DeserializeTypeInfo(HAZE_IFSTREAM& stream)
{
	// 简单实现，实际应该解析完整的序列化格式
	x_uint32 typeId = 0;
	stream >> typeId;
	return typeId;
}

HString HazeTypeRegistry::GenerateUniqueTypeName(const HazeDefineType& type, const HString& name, const HString& module,
	const TemplateDefineTypes& templateTypes, x_uint64 arrayDimension)
{
	HAZE_STRING_STREAM hss;
	
	// 模块名
	if (!module.empty())
	{
		hss << module << H_TEXT("::");
	}
	
	// 基础类型名
	hss << name;
	
	// 泛型参数
	if (templateTypes.Types.size() > 0)
	{
		hss << H_TEXT("<");
		for (size_t i = 0; i < templateTypes.Types.size(); ++i)
		{
			if (i > 0) hss << H_TEXT(",");
			if (!templateTypes.Types[i].IsDefines && templateTypes.Types[i].Type)
			{
				hss << templateTypes.Types[i].Type->BaseType.GetFullTypeName();
			}
			else
			{
				hss << H_TEXT("?");
			}
		}
		hss << H_TEXT(">");
	}
	
	// 数组维度
	if (arrayDimension > 0)
	{
		for (x_uint64 i = 0; i < arrayDimension; ++i)
		{
			hss << H_TEXT("[]");
		}
	}
	
	return hss.str();
}
