#include "HazeDefine.h"
#include "XJson.h"
#include "HazeLog.h"

#include <stdarg.h>
#include <cassert>

static XJson s_NodeNull;

inline int JsonSnprintf(char* buffer, unsigned long long size, const char* format, ...)
{
	va_list va;
	va_start(va, format);
	int result = vsnprintf_s(buffer, size, _TRUNCATE, format, va);
	va_end(va);
	return result;
}

static inline void DoubleToStringBuffer(double v, char* buffer, int size)
{
	double tmp = floor(v);
	if (tmp == v)
	{
		JsonSnprintf(buffer, size, "%ld", (long)v);
	}
	else
	{
		JsonSnprintf(buffer, size, "%g", v);
	}
}

template<typename T>
static void IntToStringBuffer(T intValue, char* str, unsigned long long size)
{
	if (str == 0 || size < 2)
	{
		return;
	}

	//str[size - 1] = '\0';

	if (intValue == 0)
	{
		str[0] = '0';
		return;
	}

	unsigned long long i = 0;
	char buf[128] = { 0 };

	T tmp = intValue < 0 ? -intValue : intValue;

	while (tmp && i < 128)
	{
		buf[i++] = (tmp % 10) + '0';
		tmp = tmp / 10;
	}

	unsigned long long needStrLen = intValue < 0 ? ++i : i;
	if (needStrLen > size)
	{
		needStrLen = size;
		i = needStrLen - 1;
	}

	str[i] = 0;
	while (true)
	{
		--i;
		if (i < 0 || buf[needStrLen - i - 1] == 0)
		{
			break;
		}

		str[i] = buf[needStrLen - i - 1];
	}

	if (i == 0)
	{
		str[i] = '-';
	}
}

template<typename T>
static void UIntToStringBuffer(T intValue, char* str, unsigned long long size)
{
	if (str == 0 || size < 2)
	{
		return;
	}

	//str[size - 1] = '\0';

	if (intValue == 0)
	{
		str[0] = '0';
		return;
	}

	unsigned long long i = 0;
	char buf[128] = { 0 };

	T tmp = intValue;

	while (tmp && i < 128)
	{
		buf[i++] = (tmp % 10) + '0';
		tmp = tmp / 10;
	}

	unsigned long long needStrLen = intValue < 0 ? ++i : i;
	if (needStrLen > size)
	{
		needStrLen = size;
		i = needStrLen - 1;
	}

	str[i] = 0;
	while (true)
	{
		--i;
		if (i < 0 || buf[needStrLen - i - 1] == 0)
		{
			break;
		}

		str[i] = buf[needStrLen - i - 1];
	}

	if (i == 0)
	{
		str[i] = '-';
	}
}

void XJson::JsonBuffer::StartRead()
{
	m_Size = m_ReadBuffer.size();
	m_Data = (char*)m_ReadBuffer.data();
	m_Offset = 0;
}

void XJson::JsonBuffer::StartWrite()
{
	m_WriteBuffer.clear();
}

XJson::JsonNodeData::JsonNodeData(DataType type)
{
	SetType(type);
}

XJson::JsonNodeData::~JsonNodeData()
{
}

void XJson::JsonNodeData::SetType(DataType type)
{
	m_Type = type;
	m_Value.Int64Value = 0;
}

void XJson::JsonNodeData::Clear()
{
	m_Value.Int64Value = 0;
}

void XJson::JsonNodeData::ToString()
{
	char buffer[64] = { 0 };
	switch (m_Type)
	{
	case DataType::None:
		m_Content = "Null";
		break;
	case DataType::Bool:
		m_Content = m_Value.BoolValue ? "true" : "false";
		break;
	case DataType::Int32:
	{
		IntToStringBuffer(m_Value.IntValue, buffer, sizeof(buffer));
		m_Content = buffer;
	}
	break;
	case DataType::UInt32:
	{
		UIntToStringBuffer(m_Value.UIntValue, buffer, sizeof(buffer));
		m_Content = buffer;
	}
	break;
	case DataType::Int64:
	{
		IntToStringBuffer(m_Value.Int64Value, buffer, sizeof(buffer));
		m_Content = buffer;
	}
	break;
	case DataType::UInt64:
	{
		UIntToStringBuffer(m_Value.UInt64Value, buffer, sizeof(buffer));
		m_Content = buffer;
	}
	break;
	case DataType::Float:
	{

		DoubleToStringBuffer(m_Value.FloatValue, buffer, sizeof(buffer));
		m_Content = buffer;
	}
	break;
	case DataType::Double:
	{
		DoubleToStringBuffer(m_Value.DoubleValue, buffer, sizeof(buffer));
		m_Content = buffer;
	}
	break;
	case DataType::String:
		break;
	default:
		m_Content.clear();
		break;
	}
}

XJson::XJson(JsonType type)
	: m_Type(type), m_ReadIndex(0), m_Length(0)
{
}

XJson::~XJson()
{
}

void XJson::operator=(bool val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		//HAZE_LOG_INFO(HAZE_TEXT("Json节点是容器类型，不能接受值!\n"));
		return;
	}

	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}

	if (!m_NodeData)
	{
		m_NodeData = std::make_unique<JsonNodeData>();
	}

	m_NodeData->SetType(JsonNodeData::DataType::Bool);
	m_NodeData->m_Value.BoolValue = val;
}

void XJson::operator=(int val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		//HAZE_LOG_INFO(HAZE_TEXT("Json节点是容器类型，不能接受值!\n"));
		return;
	}

	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}

	if (!m_NodeData)
	{
		m_NodeData = std::make_unique<JsonNodeData>();
	}

	m_NodeData->SetType(JsonNodeData::DataType::Int32);
	m_NodeData->m_Value.IntValue = val;
}

void XJson::operator=(unsigned int val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		//HAZE_LOG_INFO(HAZE_TEXT("Json节点是容器类型，不能接受值!\n"));
		return;
	}

	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}

	if (!m_NodeData)
	{
		m_NodeData = std::make_unique<JsonNodeData>();
	}

	m_NodeData->SetType(JsonNodeData::DataType::UInt32);
	m_NodeData->m_Value.UIntValue = val;
}

void XJson::operator=(long long val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		//HAZE_LOG_INFO(HAZE_TEXT("Json节点是容器类型，不能接受值!\n"));
		return;
	}

	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}

	if (!m_NodeData)
	{
		m_NodeData = std::make_unique<JsonNodeData>();
	}

	m_NodeData->SetType(JsonNodeData::DataType::Int64);
	m_NodeData->m_Value.Int64Value = val;
}

void XJson::operator=(unsigned long long val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		//HAZE_LOG_INFO(HAZE_TEXT("Json节点是容器类型，不能接受值!\n"));
		return;
	}

	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}

	if (!m_NodeData)
	{
		m_NodeData = std::make_unique<JsonNodeData>();
	}

	m_NodeData->SetType(JsonNodeData::DataType::UInt64);
	m_NodeData->m_Value.UInt64Value = val;
}

void XJson::operator=(float val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		//HAZE_LOG_INFO(HAZE_TEXT("Json节点是容器类型，不能接受值!\n"));
		return;
	}

	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}

	if (!m_NodeData)
	{
		m_NodeData = std::make_unique<JsonNodeData>();
	}

	m_NodeData->SetType(JsonNodeData::DataType::Float);
	m_NodeData->m_Value.FloatValue = val;
}

void XJson::operator=(double val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		//HAZE_LOG_INFO(HAZE_TEXT("Json节点是容器类型，不能接受值!\n"));
		return;
	}

	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}

	if (!m_NodeData)
	{
		m_NodeData = std::make_unique<JsonNodeData>();
	}

	m_NodeData->SetType(JsonNodeData::DataType::Double);
	m_NodeData->m_Value.DoubleValue = val;
}

void XJson::operator=(const char* val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		//HAZE_LOG_INFO(HAZE_TEXT("Json节点是容器类型，不能接受值!\n"));
		return;
	}

	if (m_Type != JsonType::String)
	{
		m_Type = JsonType::String;
	}

	if (!m_NodeData)
	{
		m_NodeData = std::make_unique<JsonNodeData>();
	}

	m_NodeData->SetType(JsonNodeData::DataType::String);
	m_NodeData->m_Content.clear();

	const char* ptr = nullptr;
	for (unsigned long long i = 0; i < strlen(val); ++i)
	{
		ptr = val + i;
		if (*ptr == '"' || *ptr == '\'')
		{
			m_NodeData->m_Content.push_back('\\');
		}

		m_NodeData->m_Content.push_back(*ptr);
	}
}

void XJson::operator=(const std::string& val)
{
	operator=(val.c_str());
}

XJson& XJson::SetArray(unsigned long long idx)
{
	if (m_Type != JsonType::Array)
	{
		if (m_Type == JsonType::Object)
		{
			//HAZE_LOG_INFO(HAZE_TEXT("Json节点是数组类型，不能接受对象!\n"));
		}
		m_Type = JsonType::Array;
	}
	else
	{
		assert(m_JsonValue);
	}
	if (!m_JsonValue)
	{
		m_JsonValue = std::make_unique<XJsonList>();
	}

	if (idx >= m_JsonValue->m_Childs.size())
	{
		m_JsonValue->m_Childs.resize(idx + 1);
	}

	auto& child = m_JsonValue->m_Childs[idx];
	if (!child)
	{
		child = std::make_unique<XJson>();
		m_JsonValue->m_Childs[idx] = std::move(child);
	}

	return *child;
}

XJson& XJson::SetObject(const char* str)
{
	if (!str)
	{
		return s_NodeNull;
	}

	if (m_Type != JsonType::Object)
	{
		if (m_Type == JsonType::Array)
		{
			//HAZE_LOG_INFO(HAZE_TEXT("Json节点是对象类型，不能接受数组!\n"));
		}
		m_Type = JsonType::Object;
	}

	if (!m_JsonValue)
	{
		m_JsonValue = std::make_unique<XJsonList>();
	}

	XJson* child = nullptr;
	for (unsigned long long i = 0; i < m_JsonValue->m_Childs.size(); ++i)
	{
		child = m_JsonValue->m_Childs[i].get();
		if (!child)
		{
			continue;
		}

		if (strcmp(child->KeyNodeName(), str) == 0)
		{
			return *child;
		}
	}

	auto keyNode = std::make_unique<XJson>(JsonType::String);
	*keyNode = str;
	auto newChild = std::make_unique<XJson>();
	child = newChild.get();
	child->m_KeyNameNode = std::move(keyNode);

	unsigned long long i = 0;
	for (; i < m_JsonValue->m_Childs.size(); ++i)
	{
		if (!m_JsonValue->m_Childs[i])
		{
			m_JsonValue->m_Childs[i] = std::move(newChild);
			break;
		}
	}

	if (i >= m_JsonValue->m_Childs.size())
	{
		m_JsonValue->m_Childs.push_back(std::move(newChild));
	}

	return *child;
}

const char* XJson::Data()
{
	if (m_DecodeContext && m_DecodeContext->m_Data)
	{
		if (m_ReadIndex < m_DecodeContext->m_Size)
		{
			return m_DecodeContext->m_Data + m_ReadIndex;
		}
	}

	//HAZE_LOG_INFO(HAZE_TEXT("Json节点为空，没有数据!\n"));
	return nullptr;
}

std::unique_ptr<XJson> XJson::CreateNode(char code)
{
	JsonType ctype = JsonType::None;
	switch (code)
	{
	case '"':
	case '\'':
		ctype = JsonType::String; break;
	case '{':
		ctype = JsonType::Object; break;
	case '[':
		ctype = JsonType::Array; break;
	default:
		ctype = JsonType::Number; break;
	}
	return std::make_unique<XJson>(ctype);
}

bool XJson::MakeReadContext()
{
	if (m_Type != JsonType::None)
	{
		if (m_DecodeContext && m_DecodeContext->m_Root != this)
		{
			HAZE_LOG_INFO("OpenJson warn:JsonNode is no root or empty!");
			return false;
		}
	}
	else
	{
		if (m_DecodeContext && m_DecodeContext->m_Root != this)
		{
			HAZE_LOG_INFO("OpenJson warn:JsonNode is no root or empty!");
			return false;
		};
	}
	Clear();
	m_DecodeContext = std::make_shared<JsonBuffer>();
	m_DecodeContext->m_Root = this;
	m_DecodeContext->m_Offset = 0;
	m_DecodeContext->m_ReadBuffer.clear();
	return true;
}

void XJson::AddNode(std::unique_ptr<XJson>& node)
{
	if (node)
	{
		if (m_Type != JsonType::Object && m_Type != JsonType::Array)
		{
			//HAZE_LOG_INFO(HAZE_TEXT("Json节点必须为对象或数组类型才能添加节点!\n"));
			m_Type = node->m_KeyNameNode ? JsonType::Object : JsonType::Array;
		}

		if (!m_JsonValue)
		{
			m_JsonValue = std::make_unique<XJsonList>();
		}

		m_JsonValue->Add(node);
	}
}

void XJson::Clear()
{
	if (m_NodeData)
	{
		m_NodeData.release();
	}
	if (m_KeyNameNode)
	{
		m_KeyNameNode.release();
	}
	if (m_JsonValue)
	{
		assert(m_Type == JsonType::Object || m_Type == JsonType::Array);
		m_JsonValue.release();
	}
	if (m_DecodeContext && m_DecodeContext->m_Root == this)
	{
		m_DecodeContext->m_Root = nullptr;
		m_DecodeContext.reset();
	}


	if (m_Encodecontext != 0 && m_Encodecontext->m_Root == this)
	{
		m_Encodecontext->m_Root = nullptr;
		m_Encodecontext.reset();
	}

	m_Type = JsonType::None;
	m_ReadIndex = 0;
	m_Length = 0;
}

void XJson::TrimSpace()
{
	if (!m_DecodeContext)
	{
		return;
	}

	char code = 0;
	for (unsigned long long i = m_ReadIndex; i < m_DecodeContext->m_Size; ++i)
	{
		code = m_DecodeContext->m_Data[i];
		if (code > ' ')
		{
			m_ReadIndex = i;
			break;
		}
	}
}

char XJson::GetCharCode()
{
	if (!m_DecodeContext)
	{
		return 0;
	}

	if (m_ReadIndex < m_DecodeContext->m_Size)
	{
		return m_DecodeContext->m_Data[m_ReadIndex];
	}

	return 0;
}

char XJson::GetChar()
{
	unsigned char code = GetCharCode();
	if (code <= ' ')
	{
		TrimSpace();
		code = GetCharCode();
	}
	return code;
}

char XJson::CheckCode(char charCode)
{
	char code = GetCharCode();
	if (code != charCode)
	{
		TrimSpace();
		code = GetCharCode();
		if (code != charCode) return 0;
	}

	++m_ReadIndex;

	return code;
}

unsigned long long XJson::SearchCode(char code)
{
	char* data = m_DecodeContext->m_Data;
	for (unsigned long long i = m_ReadIndex; i < m_DecodeContext->m_Size; i++)
	{
		if (data[i] == code)
		{
			if (i > 0 && data[i - 1] != '\\')
			{
				return i;
			}
		}
	}

	return (unsigned long long) - 1;
}

JsonType XJson::CodeToType(char code)
{
	JsonType ctype = JsonType::None;
	switch (code)
	{
	case '"':
	case '\'':
		ctype = JsonType::String; break;
	case '{':
		ctype = JsonType::Object; break;
	case '[':
		ctype = JsonType::Array; break;
	default:
		ctype = JsonType::Number; break;
	}
	return ctype;
}

const char* XJson::NodeDataString()
{
	if (m_Type == JsonType::String)
	{
		if (!m_NodeData)
		{
			m_NodeData = std::make_unique<JsonNodeData>(JsonNodeData::DataType::String);
			m_NodeData->m_Content = Data();
		}

		if (m_NodeData->m_Type == JsonNodeData::DataType::String)
		{
			return m_NodeData->m_Content.c_str();
		}

		m_NodeData->ToString();
		return m_NodeData->m_Content.c_str();
	}
	else if (m_Type == JsonType::Number)
	{
		if (!m_NodeData)
		{
			if (!m_DecodeContext || !m_DecodeContext->m_Data || m_Length < 1)
			{
				return nullptr;
			}

			m_NodeData = std::make_unique<JsonNodeData>(JsonNodeData::DataType::None);
			m_NodeData->m_Content = Data();
			return m_NodeData->m_Content.c_str();
		}
		if (m_NodeData)
		{
			if (m_NodeData->m_Type != JsonNodeData::DataType::None)
			{
				m_NodeData->ToString();
			}
			return m_NodeData->m_Content.c_str();
		}
	}
	else
	{
		//HAZE_LOG_INFO(HAZE_TEXT("Json节点类型不是字符串类型!\n"));
	}

	return nullptr;
}

const std::string& XJson::NodeDataStringRef()
{
	const char* data = NodeDataString();
	return m_NodeData->m_Content;
}

const char* XJson::KeyNodeName()
{
	if (m_KeyNameNode)
	{
		return m_KeyNameNode->NodeDataString();
	}

	return "";
}

const std::string& XJson::Encode()
{
	if (!m_Encodecontext)
	{
		m_Encodecontext = std::make_shared<JsonBuffer>();
		m_Encodecontext->m_Root = this;
	}

	m_Encodecontext->StartWrite();
	Write(m_Encodecontext, true);
	return m_Encodecontext->m_WriteBuffer;
}

bool XJson::Decode(const std::string& buffer)
{
	if (!m_DecodeContext)
	{
		m_DecodeContext = std::make_shared<JsonBuffer>();
		m_DecodeContext->m_Root = this;
	}

	m_DecodeContext->m_ReadBuffer = buffer;
	m_DecodeContext->StartRead();
	m_Type = CodeToType(GetChar());

	//try 
	//{
	Read(m_DecodeContext, true);
	//}
	//catch (const char* error) 
	//{
		//auto wError = String2WString(error);
		//HAZE_LOG_ERR_W("解码Json错误 <%s>!\n", wError.c_str());
	//}

	return true;
}

bool XJson::DecodeFromFile(const std::string& filePath)
{
	if (!MakeReadContext())
	{
		return false;
	}

	FILE* fp = 0;
#ifdef _MSC_VER
	fopen_s(&fp, filePath.c_str(), "rb");
#else
	fp = fopen(filePath.c_str(), "rb");
#endif
	if (fp == 0)
	{
#ifdef _MSC_VER
		char buffer[1024] = { 0 };
		strerror_s(buffer, sizeof(buffer), errno);
		HAZE_LOG_INFO("OpenJson warn:decodeFile error:%s,%s\n", buffer, filePath.c_str());
#else
		HAZE_LOG_INFO("OpenJson warn:decodeFile error:%s,%s\n", strerror(errno), filePath.c_str());
#endif
		return false;
	}
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	size_t ret = 0;
	char buff[1024 * 8] = { 0 };
	while (true)
	{
		ret = fread(buff, 1, sizeof(buff), fp);
		if (ret < 0)
		{
#ifdef _MSC_VER
			char buffer[1024] = { 0 };
			strerror_s(buffer, sizeof(buffer), errno);
			HAZE_LOG_INFO("OpenJson warn:decodeFile error:%s,%s\n", buffer, filePath.c_str());
#else
			HAZE_LOG_INFO("OpenJson warn:decodeFile error:%s,%s\n", strerror(errno), filePath.c_str());
#endif
			fclose(fp);
			return false;
		}
		else if (ret == 0) break;
		m_DecodeContext->m_ReadBuffer.append(buff, ret);
	}
	fclose(fp);

	m_DecodeContext->StartRead();
	m_Type = CodeToType(GetChar());
	try {
		Read(m_DecodeContext, true);
	}
	catch (const char* error)
	{
		HAZE_LOG_INFO("OpenJson warn:decodeFile catch exception %s", error);
	}
	return true;
}

double XJson::StringToDouble()
{
	const char* str = Data();
	double dval = 0;
	if (!str || strlen(str) == 0)
		dval = (float)(1e+300 * 1e+300) * 0.0F;
	else if (strcmp(str, "true") == 0)
		dval = 1.0;
	else if (strcmp(str, "false") == 0)
		dval = 0.0;
	else
		dval = atof(str);
	return dval;
}

float XJson::StringToFloat()
{
	return (float)StringToDouble();
}

int XJson::StringToInt32()
{
	int ret = atoi(Data());
	return ret;
}

long long XJson::StringToInt64()
{
	long long ret = atoll(Data());
	return ret;
}

void XJson::Read(std::shared_ptr<JsonBuffer> context, bool isRoot)
{
	if (m_DecodeContext)
	{
		if (isRoot)
		{
			assert(m_DecodeContext == context);
			assert(m_DecodeContext->m_Root == this);
		}
		else
		{
			assert(m_DecodeContext->m_Root != this);
			if (m_DecodeContext->m_Root == this)
			{
				return;
			}
		}
	}

	m_Length = 0;
	m_DecodeContext = context;
	m_ReadIndex = context->m_Offset;
	switch (m_Type)
	{
	case JsonType::None:
		break;
	case JsonType::String:
		ReadString();
		break;
	case JsonType::Number:
		ReadNumber();
		break;
	case JsonType::Object:
		ReadObject();
		break;
	case JsonType::Array:
		ReadArray();
		break;
	default:
		break;
	}
}

void XJson::ReadNumber()
{
	assert(m_Type == JsonType::Number);
	char code = 0;
	unsigned long long sidx = m_ReadIndex;
	unsigned long long len = m_DecodeContext->m_Size;
	char* data = m_DecodeContext->m_Data;

	for (; m_ReadIndex < len; m_ReadIndex++)
	{
		code = data[m_ReadIndex];
		if (code == ',' || code == '}' || code == ']')
		{
			m_ReadIndex--;
			break;
		}
	}

	if (m_ReadIndex < sidx)
	{
		//HAZE_LOG_ERR_W("丢失数字类型值!\n");
		return;
	}
	m_Length = m_ReadIndex - sidx + 1;
	m_ReadIndex = sidx;
}

void XJson::ReadString()
{
	assert(m_Type == JsonType::String);
	unsigned char code = '"';
	if (!CheckCode(code))
	{
		code = '\'';
		if (!CheckCode(code))
		{
			//HAZE_LOG_ERR_W("丢失<'\"'>或<\"'\">!\n");
			return;
		}
	}
	unsigned long long sidx = m_ReadIndex;
	unsigned long long eidx = SearchCode(code);
	if (eidx < 0)
	{
		//HAZE_LOG_ERR_W("丢失<'\"'>或<\"'\">!\n");
		return;
	}
	m_ReadIndex = sidx;
	m_Length = eidx - sidx + 1;
	m_DecodeContext->m_Data[eidx] = 0;
}

void XJson::ReadObject()
{
	assert(m_Type == JsonType::Object);

	if (!CheckCode('{'))
	{
		//HAZE_LOG_ERR_W("丢失<'{'>!\n");
		return;
	}

	char code = 0;
	unsigned long long oidx = m_ReadIndex;
	while (m_ReadIndex < m_DecodeContext->m_Size)
	{
		code = GetChar();
		if (code == 0)
		{
			//HAZE_LOG_ERR_W("丢失<'}'>!\n");
			return;
		}

		if (CheckCode('}'))
		{
			break;
		}

		auto keyNode = CreateNode(code);
		if (keyNode->m_Type != JsonType::String)
		{
			//HAZE_LOG_ERR_W("丢失Json的Key!\n");
			return;
		}
		m_DecodeContext->m_Offset = m_ReadIndex;
		keyNode->Read(m_DecodeContext);
		m_ReadIndex = keyNode->m_ReadIndex + keyNode->m_Length;

		if (!CheckCode(':'))
		{
			//HAZE_LOG_ERR_W("丢失Json的<':'>!\n");
			return;
		}

		code = GetChar();
		auto valNode = CreateNode(code);
		valNode->m_KeyNameNode = std::move(keyNode);
		m_DecodeContext->m_Offset = m_ReadIndex;
		valNode->Read(m_DecodeContext);
		m_ReadIndex = valNode->m_ReadIndex + valNode->m_Length;
		AddNode(valNode);

		if (CheckCode('}'))
		{
			m_DecodeContext->m_Data[m_ReadIndex - 1] = 0;
			break;
		}
		if (!CheckCode(','))
		{
			//HAZE_LOG_ERR_W("丢失Json的<','>!\n");
			return;
		}
		m_DecodeContext->m_Data[m_ReadIndex - 1] = 0;
	}

	m_Length = m_ReadIndex - oidx;
	m_ReadIndex = oidx;
}

void XJson::ReadArray()
{
	assert(m_Type == JsonType::Array);
	if (!CheckCode('['))
	{
		//HAZE_LOG_ERR_W("丢失数组类型的<'['>!\n");
		return;
	}
	unsigned char code = 0;
	unsigned long long oidx = m_ReadIndex;
	while (m_ReadIndex < m_DecodeContext->m_Size)
	{
		code = GetChar();
		if (code == 0)
		{
			//HAZE_LOG_ERR_W("丢失数组类型的<']'>!\n");
			return;
		}
		if (CheckCode(']')) break;
		auto valNode = CreateNode(code);
		m_DecodeContext->m_Offset = m_ReadIndex;
		valNode->Read(m_DecodeContext);
		m_ReadIndex = valNode->m_ReadIndex + valNode->m_Length;
		AddNode(valNode);

		if (CheckCode(']'))
		{
			m_DecodeContext->m_Data[m_ReadIndex - 1] = 0;
			break;
		}
		if (!CheckCode(','))
		{
			//HAZE_LOG_ERR_W("丢失Json的<','>!\n");
			return;
		}
		m_DecodeContext->m_Data[m_ReadIndex - 1] = 0;
	}

	m_Length = m_ReadIndex - oidx;
	m_ReadIndex = oidx;
}


void XJson::Write(std::shared_ptr<JsonBuffer> context, bool isRoot)
{
	if (m_Encodecontext)
	{
		if (isRoot)
		{
			assert(m_Encodecontext == context);
			assert(m_Encodecontext->m_Root == this);
		}
		else
		{
			assert(m_Encodecontext->m_Root != this);
			if (m_Encodecontext->m_Root == this)
			{
				return;
			}
		}
	}

	m_Encodecontext = context;

	switch (m_Type)
	{
	case JsonType::None:
		break;
	case JsonType::String:
		WriteString();
		break;
	case JsonType::Number:
		WriteNumber();
		break;
	case JsonType::Object:
		WriteObject();
		break;
	case JsonType::Array:
		WriteArray();
		break;
	default:
		break;
	}
}

void XJson::WriteNumber()
{
	assert(m_Type == JsonType::Number);

	if (m_KeyNameNode)
	{
		m_Encodecontext->m_WriteBuffer.append(std::string("\"") + KeyNodeName() + "\":");
	}
	if (m_NodeData)
	{
		m_NodeData->ToString();
		m_Encodecontext->m_WriteBuffer.append(m_NodeData->m_Content);
	}
	else
	{
		m_Encodecontext->m_WriteBuffer.append(Data());
	}
}

void XJson::WriteString()
{
	assert(m_Type == JsonType::String);

	if (m_KeyNameNode)
	{
		m_Encodecontext->m_WriteBuffer.append(std::string("\"") + KeyNodeName() + "\":");
	}
	m_Encodecontext->m_WriteBuffer.append(std::string("\"") + NodeDataString() + "\"");
}

void XJson::WriteObject()
{
	assert(m_Type == JsonType::Object);

	if (m_KeyNameNode)
	{
		m_Encodecontext->m_WriteBuffer.append(std::string("\"") + KeyNodeName() + "\":{");
	}
	else
	{
		m_Encodecontext->m_WriteBuffer.append("{");
	}

	if (m_JsonValue != 0)
	{
		size_t idx = 0;
		size_t size = m_JsonValue->Size();
		for (size_t i = 0; i < size; ++i)
		{
			if (!(*m_JsonValue)[i]) continue;
			if (idx > 0)
			{
				m_Encodecontext->m_WriteBuffer.append(",");
			}
			(*m_JsonValue)[i]->Write(m_Encodecontext);
			++idx;
		}
	}

	m_Encodecontext->m_WriteBuffer.append("}");
}

void XJson::WriteArray()
{
	assert(m_Type == JsonType::Array);

	if (m_KeyNameNode)
	{
		m_Encodecontext->m_WriteBuffer.append(std::string("\"") + KeyNodeName() + "\":[");
	}
	else
	{
		m_Encodecontext->m_WriteBuffer.append("[");
	}

	if (m_JsonValue != 0)
	{
		size_t idx = 0;
		size_t size = m_JsonValue->Size();
		for (size_t i = 0; i < size; ++i)
		{
			if (!(*m_JsonValue)[i]) continue;
			if (idx > 0)
			{
				m_Encodecontext->m_WriteBuffer.append(",");
			}
			(*m_JsonValue)[i]->Write(m_Encodecontext);
			++idx;
		}
	}

	m_Encodecontext->m_WriteBuffer.append("]");
}