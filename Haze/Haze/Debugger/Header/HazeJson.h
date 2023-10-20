#pragma once

#include "Haze.h"

//ÐÞ¸Ä×ÔOpenJson
enum class JsonType
{
	None,
	String,
	Number,
	Object,
	Array,
};

class HazeJson
{
private:
	class HazeJsonList
	{
		friend class HazeJson;
	public:
		HazeJsonList() {}

		~HazeJsonList() {}

		void Clear() { m_Childs.clear(); }

		bool Empty() { return m_Childs.empty(); }
		
		uint64 Size() { return m_Childs.size(); }
		
		void Add(std::unique_ptr<HazeJson>& node) { m_Childs.push_back(std::move(node)); }

		HazeJson* operator[](uint64 idx) { return m_Childs[idx].get(); }
		
		bool Remove(HazeJson* node)
		{
			for (auto iter = m_Childs.begin(); iter != m_Childs.end(); iter++)
			{
				if (iter->get() == node)
				{
					m_Childs.erase(iter);
					return true;
				}
			}

			return false;
		}

	private:
		std::vector<std::unique_ptr<HazeJson>> m_Childs;
	};

	class JsonBuffer
	{
		friend class HazeJson;
	public:
		JsonBuffer() : m_Root(nullptr), m_Offset(0), m_Data(nullptr), m_Size(0)
		{}
		
		~JsonBuffer() {}

		void StartRead();
		
		void StartWrite();

	private:
		HazeJson* m_Root;

		char* m_Data;
		size_t m_Size;
		size_t m_Offset;

		std::string m_ReadBuffer;
		std::string m_WriteBuffer;
	};

	class JsonNodeData
	{
		friend class HazeJson;
	public:
		enum class DataType
		{
			None = 0,
			Bool,
			Int32,
			UInt32,
			Int64,
			UInt64,
			Float,
			Double,
			String
		};

		JsonNodeData(DataType type = DataType::None);

		~JsonNodeData();

		void Clear();
		
		void ToString();
		
		void SetType(DataType type);

	private:
		union
		{
			bool BoolValue;
			int IntValue;
			uint32 UIntValue;
			int64 Int64Value;
			uint64 UInt64Value;
			float FloatValue;
			double DoubleValue;
		} m_Value;

		DataType m_Type;
		std::string m_Content;
	};

public:
	HazeJson(JsonType type = JsonType::None);

	~HazeJson();

	void operator=(bool val);
	
	void operator=(int val);
	
	void operator=(uint32 val);
	
	void operator=(int64 val);
	
	void operator=(uint64 val);
	
	void operator=(float val);

	void operator=(double val);
	
	void operator=(const char* val);
	
	void operator=(const std::string& val);

	HazeJson& operator[](int idx) { return SetArray(idx); }

	HazeJson& operator[](uint64 idx) { return SetArray(idx); }

	HazeJson& operator[](const char* str) { return SetObject(str); }

	HazeJson& operator[](const std::string& str) { return SetObject(str.c_str()); }

	size_t Size() { return m_JsonValue ? m_JsonValue->Size() : 0; }

	bool Empty() { return m_JsonValue ? m_JsonValue->Empty() : true; }

	std::unique_ptr<HazeJson> CreateNode(char code);

	void AddNode(std::unique_ptr<HazeJson>& node);

	void TrimSpace();

	char GetCharCode();

	char GetChar();

	char CheckCode(char charCode);

	usize SearchCode(char code);

	JsonType CodeToType(char code);
	
	const char* NodeDataString();

	const char* KeyNodeName();
	
	const char* Data();

	const std::string& Encode();

	bool Decode(const std::string& buffer);

private:
	void Read(std::shared_ptr<JsonBuffer> context, bool isRoot = false);

	void ReadNumber();
	
	void ReadString();
	
	void ReadObject();
	
	void ReadArray();

	void Write(std::shared_ptr<JsonBuffer> context, bool isRoot = false);

	void WriteNumber();
	
	void WriteString();
	
	void WriteObject();
	
	void WriteArray();

	HazeJson& SetArray(usize idx);

	HazeJson& SetObject(const char* str);

private:
	JsonType m_Type;
	usize m_ReadIndex;
	usize m_Length;

	std::unique_ptr<HazeJson> m_KeyNameNode;
	std::unique_ptr<HazeJsonList> m_JsonValue;
	std::unique_ptr<JsonNodeData> m_NodeData;

	std::shared_ptr<JsonBuffer> m_DecodeContext;
	std::shared_ptr<JsonBuffer> m_Encodecontext;
};
