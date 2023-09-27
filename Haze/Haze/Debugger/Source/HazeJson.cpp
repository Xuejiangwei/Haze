#include "HazeJson.h"
#include <stdarg.h>

static HazeJson s_NodeNull;

inline int SNPRINTF(char* buffer, size_t size, const char* format, ...)
{
	va_list va;
	va_start(va, format);
	int result = vsnprintf_s(buffer, size, _TRUNCATE, format, va);
	va_end(va);
	return result;
}

static inline void doubleToStr(double v, char* buffer, int size)
{
	double tmp = floor(v);
	if (tmp == v)
		SNPRINTF(buffer, size, "%ld", (long)v);
	else
		SNPRINTF(buffer, size, "%g", v);
}

static void int32ToStr(int32_t n, char* str, size_t size)
{
	if (str == 0 || size < 1) return;
	str[size - 1] = 0;
	if (size < 2) return;
	if (n == 0)
	{
		str[0] = '0';
		return;
	}
	size_t i = 0;
	char buf[128] = { 0 };
	int32_t tmp = n < 0 ? -n : n;
	while (tmp && i < 128)
	{
		buf[i++] = (tmp % 10) + '0';
		tmp = tmp / 10;
	}
	size_t len = n < 0 ? ++i : i;
	if (len > size)
	{
		len = size;
		i = len - 1;
	}
	str[i] = 0;
	while (1)
	{
		--i;
		if (i < 0 || buf[len - i - 1] == 0) break;
		str[i] = buf[len - i - 1];
	}
	if (i == 0) str[i] = '-';
}

static void int64ToStr(int64_t n, char* str, size_t size)
{
	if (str == 0 || size < 1) return;
	str[size - 1] = 0;
	if (size < 2) return;
	if (n == 0)
	{
		str[0] = '0';
		return;
	}
	size_t i = 0;
	char buf[128] = { 0 };
	int64_t tmp = n < 0 ? -n : n;
	while (tmp && i < 128)
	{
		buf[i++] = (tmp % 10) + '0';
		tmp = tmp / 10;
	}
	size_t len = n < 0 ? ++i : i;
	if (len > size)
	{
		len = size;
		i = len - 1;
	}
	str[i] = 0;
	while (1)
	{
		--i;
		if (i < 0 || buf[len - i - 1] == 0) break;
		str[i] = buf[len - i - 1];
	}
	if (i == 0) str[i] = '-';
}

void HazeJson::Context::StartRead()
{
	m_Size = m_ReadBuffer.size();
	m_Data = (char*)m_ReadBuffer.data();
	m_Offset = 0;
}

void HazeJson::Context::StartWrite()
{
	m_WriteBuffer.clear();
}

HazeJson::Segment::Segment(SegmentType type)
{
	SetType(type);
}

HazeJson::Segment::~Segment()
{
}

void HazeJson::Segment::SetType(SegmentType type)
{
	m_Type = type;
	m_Value.int64_ = 0;
}

void HazeJson::Segment::Clear()
{
	m_Value.int64_ = 0;
}

void HazeJson::Segment::ToString()
{
	switch (m_Type)
	{
	case SegmentType::None:
		m_Content = "null";
		break;
	case SegmentType::Bool:
		m_Content = m_Value.bool_ ? "true" : "false";
		break;
	case SegmentType::Int32:
	{
		char buffer[64] = { 0 };
		int32ToStr(m_Value.int32_, buffer, sizeof(buffer));
		m_Content = buffer;
	}
	break;
	case SegmentType::Int64:
	{
		char buffer[64] = { 0 };
		int64ToStr(m_Value.int64_, buffer, sizeof(buffer));
		m_Content = buffer;
	}
	break;
	case SegmentType::Double:
	{
		char buffer[64] = { 0 };
		doubleToStr(m_Value.double_, buffer, sizeof(buffer));
		m_Content = buffer;
	}
	break;
	case SegmentType::String:
		break;
	default:
		m_Content.clear();
		break;
	}
}

HazeJson::HazeJson(JsonType type)
	: m_Type(type), m_Index(0), m_Length(0)
{
}

HazeJson::~HazeJson()
{
}

void HazeJson::Log(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	char tmp[1024] = { 0 };
	vsnprintf(tmp, sizeof(tmp), format, ap);
	va_end(ap);
	printf("HazeJson WARN:%s\n", tmp);
}

void HazeJson::operator=(bool val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		//Log("JsonNode is a container, not element");
		return;
	}
	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}
	if (!m_JsonSegment)
	{
		m_JsonSegment = std::make_unique<Segment>();
	}

	m_JsonSegment->SetType(Segment::SegmentType::Bool);
	m_JsonSegment->m_Value.bool_ = val;
}

void HazeJson::operator=(int32_t val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		Log("JsonNode is a container, not element");
		return;
	}
	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}
	if (!m_JsonSegment)
	{
		m_JsonSegment = std::make_unique<Segment>();
	}

	m_JsonSegment->SetType(Segment::SegmentType::Int32);
	m_JsonSegment->m_Value.int32_ = val;
}

void HazeJson::operator=(uint32_t val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		Log("JsonNode is a container, not element");
		return;
	}
	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}
	if (!m_JsonSegment)
	{
		m_JsonSegment = std::make_unique<Segment>();
	}

	m_JsonSegment->SetType(Segment::SegmentType::Int32);
	m_JsonSegment->m_Value.int32_ = val;
}

void HazeJson::operator=(int64_t val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		Log("JsonNode is a container, not element");
		return;
	}
	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}
	if (!m_JsonSegment)
	{
		m_JsonSegment = std::make_unique<Segment>();
	}

	m_JsonSegment->SetType(Segment::SegmentType::Int64);
	m_JsonSegment->m_Value.int64_ = val;
}

void HazeJson::operator=(uint64_t val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		Log("JsonNode is a container, not element");
		return;
	}
	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}
	if (!m_JsonSegment)
	{
		m_JsonSegment = std::make_unique<Segment>();
	}

	m_JsonSegment->SetType(Segment::SegmentType::Int64);
	m_JsonSegment->m_Value.int64_ = val;
}

void HazeJson::operator=(double val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		Log("JsonNode is a container, not element");
		return;
	}
	if (m_Type != JsonType::Number)
	{
		m_Type = JsonType::Number;
	}
	if (!m_JsonSegment)
	{
		m_JsonSegment = std::make_unique<Segment>();
	}

	m_JsonSegment->SetType(Segment::SegmentType::Double);
	m_JsonSegment->m_Value.double_ = val;
}

void HazeJson::operator=(const char* val)
{
	if (m_Type == JsonType::Object || m_Type == JsonType::Array)
	{
		Log("JsonNode is a container, not element");
		return;
	}
	if (m_Type != JsonType::String)
	{
		m_Type = JsonType::String;
	}
	if (!m_JsonSegment)
	{
		m_JsonSegment = std::make_unique<Segment>();
	}

	m_JsonSegment->SetType(Segment::SegmentType::String);
	m_JsonSegment->m_Content.clear();

	const char* ptr = 0;
	for (size_t i = 0; i < strlen(val); ++i)
	{
		ptr = val + i;
		if (*ptr == '"' || *ptr == '\'')
		{
			m_JsonSegment->m_Content.push_back('\\');
		}
		m_JsonSegment->m_Content.push_back(*ptr);
	}
}

void HazeJson::operator=(const std::string& val)
{
	operator=(val.c_str());
}

HazeJson& HazeJson::SetArray(uint64 idx)
{
	if (m_Type != JsonType::Array)
	{
		if (m_Type == JsonType::Object)
		{
			//Log("JsonNode must be ARRAY, not OBJECT");
		}
		m_Type = JsonType::Array;
	}
	else
	{
		assert(m_Box);
	}
	if (!m_Box)
	{
		m_Box = std::make_unique<HazeJsonList>();
	}
	
	if (idx >= m_Box->m_Childs.size())
	{
		m_Box->m_Childs.resize(idx + 1);
	}
	
	auto& child = m_Box->m_Childs[idx];
	if (!child)
	{
		child = std::make_unique<HazeJson>();
		m_Box->m_Childs[idx] = std::move(child);
	}

	return *child;
}

HazeJson& HazeJson::SetObject(const char* str)
{
	if (!str)
	{
		return s_NodeNull;
	}
	if (m_Type != JsonType::Object)
	{
		if (m_Type == JsonType::Array)
		{
			//Log("JsonNode must be OBJECT, not ARRAY");
		}
		m_Type = JsonType::Object;
	}
	else
	{
		assert(m_Box);
	}
	if (!m_Box) m_Box = std::make_unique<HazeJsonList>();

	HazeJson* child = nullptr;
	for (size_t i = 0; i < m_Box->m_Childs.size(); ++i)
	{
		child = m_Box->m_Childs[i].get();
		if (!child) continue;
		if (strcmp(child->KeyName(), str) == 0)
		{
			return *child;
		}
	}
	auto keyNode = std::make_unique<HazeJson>(JsonType::String);
	*keyNode = str;
	auto newChild = std::make_unique<HazeJson>();
	child = newChild.get();
	child->m_KeyName = std::move(keyNode);

	size_t i = 0;
	for (; i < m_Box->m_Childs.size(); ++i)
	{
		if (!m_Box->m_Childs[i])
		{
			m_Box->m_Childs[i] = std::move(newChild);
			break;
		}
	}

	if (i >= m_Box->m_Childs.size())
	{
		m_Box->m_Childs.push_back(std::move(newChild));
	}
	return *child;
}

const char* HazeJson::data()
{
	if (m_Context && m_Context->m_Data)
	{
		if (m_Index < m_Context->m_Size)
		{
			return m_Context->m_Data + m_Index;
		}
	}
	Log("JsonNode is Empty");
	return nullptr;
}

std::unique_ptr<HazeJson> HazeJson::CreateNode(uint8 code)
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
	return std::make_unique<HazeJson>(ctype);
}

void HazeJson::AddNode(std::unique_ptr<HazeJson>& node)
{
	if (!node) return;

	if (m_Type != JsonType::Object && m_Type != JsonType::Array)
	{
		Log("JsonNode must be OBJECT or ARRAY");
		m_Type = node->m_KeyName ? JsonType::Object : JsonType::Array;
	}

	if (!m_Box) m_Box = std::make_unique<HazeJsonList>();
	m_Box->Add(node);
}

void HazeJson::TrimSpace()
{
	if (!m_Context)
	{
		return;
	}
	
	char code = 0;
	for (size_t i = m_Index; i < m_Context->m_Size; ++i)
	{
		code = m_Context->m_Data[i];
		if (code > ' ')
		{
			m_Index = i; break;
		}
	}
}

uint8 HazeJson::GetCharCode()
{
	if (!m_Context)
	{
		return 0;
	}
	
	if (m_Index < m_Context->m_Size)
	{
		return (uint8)m_Context->m_Data[m_Index];
	}

	return 0;
}

uint8 HazeJson::GetChar()
{
	uint8 code = GetCharCode();
	if (code <= ' ')
	{
		TrimSpace();
		code = GetCharCode();
	}
	return code;
}

uint8 HazeJson::CheckCode(uint8 charCode)
{
	uint8 code = GetCharCode();
	if (code != charCode)
	{
		TrimSpace();
		code = GetCharCode();
		if (code != charCode) return 0;
	}
	++m_Index;
	return code;
}

uint64 HazeJson::SearchCode(uint8 code)
{
	char* data = m_Context->m_Data;
	for (size_t i = m_Index; i < m_Context->m_Size; i++)
	{
		if (data[i] == code)
		{
			if (i > 0 && data[i - 1] != '\\') return i;
		}
	}

	return (uint64)-1;
}

JsonType HazeJson::CodeToType(uint8 code)
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

const char* HazeJson::SegmentString()
{
	if (m_Type == JsonType::String)
	{
		if (!m_JsonSegment)
		{
			m_JsonSegment = std::make_unique<Segment>(Segment::SegmentType::String);
			m_JsonSegment->m_Content = data();
		}
		if (m_JsonSegment->m_Type == Segment::SegmentType::String)
		{
			return m_JsonSegment->m_Content.c_str();
		}
		m_JsonSegment->ToString();
		return m_JsonSegment->m_Content.c_str();
	}
	else if (m_Type == JsonType::Number)
	{
		if (!m_JsonSegment)
		{
			if (!m_Context || !m_Context->m_Data || m_Length < 1)
			{
				return nullptr;
			}
			m_JsonSegment = std::make_unique<Segment>(Segment::SegmentType::None);
			m_JsonSegment->m_Content = data();
			return m_JsonSegment->m_Content.c_str();
		}
		if (m_JsonSegment)
		{
			if (m_JsonSegment->m_Type != Segment::SegmentType::None)
			{
				m_JsonSegment->ToString();
			}
			return m_JsonSegment->m_Content.c_str();
		}
	}
	else
	{
		//Log("JsonNode is no STRING");
	}
	return nullptr;
}

const char* HazeJson::KeyName()
{
	if (m_KeyName)
	{
		return m_KeyName->SegmentString();
	}

	return "";
}

const std::string& HazeJson::Encode()
{
	if (!m_Writecontext)
	{
		m_Writecontext = std::make_shared<Context>();
		m_Writecontext->m_Root = this;
	}

	m_Writecontext->StartWrite();
	Write(m_Writecontext, true);
	return m_Writecontext->m_WriteBuffer;
}

bool HazeJson::Decode(const std::string& buffer)
{
	//if (!makeRContext()) return false;
	m_Context->m_ReadBuffer = buffer;
	m_Context->StartRead();
	m_Type = CodeToType(GetChar());
	try 
	{
		Read(m_Context, true);
	}
	catch (const char* error) 
	{
		printf("HazeJson warn:decode catch exception %s", error);
	}
	return true;
}

void HazeJson::Read(std::shared_ptr<Context> context, bool isRoot)
{
	if (m_Context)
	{
		if (isRoot)
		{
			assert(m_Context == context);
			assert(m_Context->m_Root == this);
		}
		else
		{
			assert(m_Context->m_Root != this);
			if (m_Context->m_Root == this) return;
		}
	}

	m_Length = 0;
	m_Context = context;
	m_Index = context->m_Offset;
	switch (m_Type)
	{
	case JsonType::None:
		break;
	case JsonType::String:
		ReadString(); break;
	case JsonType::Number:
		ReadNumber(); break;
	case JsonType::Object:
		ReadObject(); break;
	case JsonType::Array:
		ReadArray(); break;
	default:
		break;
	}
}

void HazeJson::ReadNumber()
{
	assert(m_Type == JsonType::Number);
	unsigned char code = 0;
	size_t sidx = m_Index;
	size_t len = m_Context->m_Size;
	char* data = m_Context->m_Data;

	for (; m_Index < len; m_Index++)
	{
		code = data[m_Index];
		if (code == ',' || code == '}' || code == ']')
		{
			m_Index--;
			break;
		}
	}

	if (m_Index < sidx)
	{
		//throwError("lost number value");
		return;
	}
	m_Length = m_Index - sidx + 1;
	m_Index = sidx;
}

void HazeJson::ReadString()
{
	assert(m_Type == JsonType::String);
	unsigned char code = '"';
	if (!CheckCode(code))
	{
		code = '\'';
		if (!CheckCode(code))
		{
			//throwError("lost '\"' or \"'\"");
			return;
		}
	}
	size_t sidx = m_Index;
	size_t eidx = SearchCode(code);
	if (eidx < 0)
	{
		//throwError("lost '\"' or \"'\"");
		return;
	}
	m_Index = sidx;
	m_Length = eidx - sidx + 1;
	m_Context->m_Data[eidx] = 0;
}

void HazeJson::ReadObject()
{
	assert(m_Type == JsonType::Object);
	if (!CheckCode('{'))
	{
		//throwError("lost '{'");
		return;
	}
	unsigned char code = 0;
	size_t oidx = m_Index;
	while (m_Index < m_Context->m_Size)
	{
		code = GetChar();
		if (code == 0)
		{
			//throwError("lost '}'");
			return;
		}
		
		if (CheckCode('}')) break;

		auto keyNode = CreateNode(code);
		if (keyNode->m_Type != JsonType::String)
		{
			//throwError("lost key");
			return;
		}
		m_Context->m_Offset = m_Index;
		keyNode->Read(m_Context);
		m_Index = keyNode->m_Index + keyNode->m_Length;
		if (!CheckCode(':'))
		{
			//throwError("lost ':'");
			return;
		}

		code = GetChar();
		auto valNode = CreateNode(code);
		valNode->m_KeyName = std::move(keyNode);
		m_Context->m_Offset = m_Index;
		valNode->Read(m_Context);
		m_Index = valNode->m_Index + valNode->m_Length;
		AddNode(valNode);

		if (CheckCode('}'))
		{
			m_Context->m_Data[m_Index - 1] = 0;
			break;
		}
		if (!CheckCode(','))
		{
			//throwError("lost ','");
			return;
		}
		m_Context->m_Data[m_Index - 1] = 0;
	}

	m_Length = m_Index - oidx;
	m_Index = oidx;
}

void HazeJson::ReadArray()
{
	assert(m_Type == JsonType::Array);
	if (!CheckCode('['))
	{
		//throwError("lost '['");
		return;
	}
	uint8 code = 0;
	size_t oidx = m_Index;
	while (m_Index < m_Context->m_Size)
	{
		code = GetChar();
		if (code == 0)
		{
			//throwError("lost ']'");
			return;
		}
		if (CheckCode(']')) break;
		auto valNode = CreateNode(code);
		m_Context->m_Offset = m_Index;
		valNode->Read(m_Context);
		m_Index = valNode->m_Index + valNode->m_Length;
		AddNode(valNode);

		if (CheckCode(']'))
		{
			m_Context->m_Data[m_Index - 1] = 0;
			break;
		}
		if (!CheckCode(','))
		{
			//throwError("lost ','");
			return;
		}
		m_Context->m_Data[m_Index - 1] = 0;
	}

	m_Length = m_Index - oidx;
	m_Index = oidx;
}


void HazeJson::Write(std::shared_ptr<Context> context, bool isRoot)
{
	if (m_Writecontext)
	{
		if (isRoot)
		{
			assert(m_Writecontext == context);
			assert(m_Writecontext->m_Root == this);
		}
		else
		{
			assert(m_Writecontext->m_Root != this);
			if (m_Writecontext->m_Root == this) return;
		}
	}

	m_Writecontext = context;

	switch (m_Type)
	{
	case JsonType::None:
		break;
	case JsonType::String:
		WriteString(); break;
	case JsonType::Number:
		WriteNumber(); break;
	case JsonType::Object:
		WriteObject(); break;
	case JsonType::Array:
		WriteArray(); break;
	default:
		break;
	}
}

void HazeJson::WriteNumber()
{
	assert(m_Type == JsonType::Number);

	if (m_KeyName)
	{
		m_Writecontext->m_WriteBuffer.append(std::string("\"") + KeyName() + "\":");
	}
	if (m_JsonSegment)
	{
		m_JsonSegment->ToString();
		m_Writecontext->m_WriteBuffer.append(m_JsonSegment->m_Content);
	}
	else
	{
		m_Writecontext->m_WriteBuffer.append(data());
	}
}

void HazeJson::WriteString()
{
	assert(m_Type == JsonType::String);

	if (m_KeyName)
	{
		m_Writecontext->m_WriteBuffer.append(std::string("\"") + KeyName() + "\":");
	}
	m_Writecontext->m_WriteBuffer.append(std::string("\"") + SegmentString() + "\"");
}

void HazeJson::WriteObject()
{
	assert(m_Type == JsonType::Object);

	if (m_KeyName)
	{
		m_Writecontext->m_WriteBuffer.append(std::string("\"") + KeyName() + "\":{");
	}
	else
	{
		m_Writecontext->m_WriteBuffer.append("{");
	}

	if (m_Box != 0)
	{
		size_t idx = 0;
		size_t size = m_Box->Size();
		for (size_t i = 0; i < size; ++i)
		{
			if (!(*m_Box)[i]) continue;
			if (idx > 0)
			{
				m_Writecontext->m_WriteBuffer.append(",");
			}
			(*m_Box)[i]->Write(m_Writecontext);
			++idx;
		}
	}

	m_Writecontext->m_WriteBuffer.append("}");
}

void HazeJson::WriteArray()
{
	assert(m_Type == JsonType::Array);

	if (m_KeyName)
	{
		m_Writecontext->m_WriteBuffer.append(std::string("\"") + KeyName() + "\":[");
	}
	else
	{
		m_Writecontext->m_WriteBuffer.append("[");
	}

	if (m_Box != 0)
	{
		size_t idx = 0;
		size_t size = m_Box->Size();
		for (size_t i = 0; i < size; ++i)
		{
			if (!(*m_Box)[i]) continue;
			if (idx > 0)
			{
				m_Writecontext->m_WriteBuffer.append(",");
			}
			(*m_Box)[i]->Write(m_Writecontext);
			++idx;
		}
	}

	m_Writecontext->m_WriteBuffer.append("]");
}