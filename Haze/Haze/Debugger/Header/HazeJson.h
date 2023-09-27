#pragma once

#include "Haze.h"

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
		
		size_t Size() { return m_Childs.size(); }
		
		void Add(std::unique_ptr<HazeJson>& node) { m_Childs.push_back(std::move(node)); }

		HazeJson* operator[](size_t idx) { return m_Childs[idx].get(); }
		
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

	class Context
	{
		friend class HazeJson;

		char* m_Data;
		size_t m_Size;
		size_t m_Offset;

		HazeJson* m_Root;
		std::string m_ReadBuffer;
		std::string m_WriteBuffer;

		std::string stringNull_;
	public:
		Context() : m_Root(nullptr), m_Offset(0), m_Data(nullptr), m_Size(0)
		{}
		
		~Context() {}

		void StartRead();
		
		void StartWrite();
	};

	class Segment
	{
	public:
		enum class SegmentType
		{
			None = 0,
			Bool,
			Int32,
			Int64,
			Double,
			String
		};
		
		SegmentType m_Type;
		std::string m_Content;

		union 
		{
			bool bool_;
			int32_t int32_;
			int64_t int64_;
			double double_;
		} m_Value;

		Segment(SegmentType type = SegmentType::None);

		~Segment();

		void Clear();
		
		void ToString();
		
		void SetType(SegmentType type);
	};

public:
	HazeJson(JsonType type = JsonType::None);

	~HazeJson();

	void operator=(bool val);
	
	void operator=(int32_t val);
	
	void operator=(uint32_t val);
	
	void operator=(int64_t val);
	
	void operator=(uint64_t val);
	
	void operator=(double val);
	
	void operator=(const char* val);
	
	void operator=(const std::string& val);

	HazeJson& operator[](int idx) { return SetArray(idx); }

	HazeJson& operator[](uint64 idx) { return SetArray(idx); }

	HazeJson& operator[](const char* str) { return SetObject(str); }

	HazeJson& operator[](const std::string& str) { return SetObject(str.c_str()); }

	size_t Size() { return m_Box ? m_Box->Size() : 0; }

	bool Empty() { return m_Box ? m_Box->Empty() : true; }

	std::unique_ptr<HazeJson> CreateNode(uint8 code);

	void AddNode(std::unique_ptr<HazeJson>& node);

	void TrimSpace();

	uint8 GetCharCode();

	uint8 GetChar();

	uint8 CheckCode(uint8 charCode);

	uint64 SearchCode(uint8 code);

	JsonType CodeToType(uint8 code);
	
	const char* SegmentString();

	const char* KeyName();
	
	const char* data();

	const std::string& Encode();

	bool Decode(const std::string& buffer);

private:
	void Read(std::shared_ptr<Context> context, bool isRoot = false);

	void ReadNumber();
	
	void ReadString();
	
	void ReadObject();
	
	void ReadArray();

	void Write(std::shared_ptr<Context> context, bool isRoot = false);

	void WriteNumber();
	
	void WriteString();
	
	void WriteObject();
	
	void WriteArray();

	HazeJson& SetArray(uint64 idx);

	HazeJson& SetObject(const char* str);

	static void Log(const char* format, ...);
private:
	JsonType m_Type;
	std::unique_ptr<HazeJsonList> m_Box;
	std::unique_ptr<HazeJson> m_KeyName;
	std::unique_ptr<Segment> m_JsonSegment;

	std::shared_ptr<Context> m_Context;
	std::shared_ptr<Context> m_Writecontext;
	size_t m_Index;
	size_t m_Length;
};
