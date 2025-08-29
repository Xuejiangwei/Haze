#include "HazePch.h"

HazeString::HazeString(const hchar* data) : m_Hash(0)
{
	if (data)
	{
		m_Length = std::char_traits<hchar>::length(data);
		m_Data = (hchar*)malloc((m_Length + 1) * sizeof(hchar));
		memcpy(m_Data, data, m_Length * sizeof(hchar));
		m_Data[m_Length] = 0;
	}
	else
	{
		m_Data = nullptr;
		m_Length = 0;
	}
}

HazeString::HazeString(HazeString&& str)
{
	memcpy(this, &str, sizeof(str));
	str.m_Data = nullptr;
}

HazeString::HazeString(const STD_Str& str)
{
	if (!str.empty())
	{
		m_Length = str.length();
		m_Data = (hchar*)malloc((m_Length + 1) * sizeof(hchar));
		memcpy(m_Data, str.c_str(), m_Length * sizeof(hchar));
		m_Data[m_Length] = 0;
	}
	else
	{
		m_Data = nullptr;
		m_Length = 0;
	}
}

HazeString::~HazeString()
{
	free(m_Data);
}

HazeString& HazeString::operator=(const STD_Str& str)
{
	decltype(m_Data) freeData = m_Data;

	m_Data = (hchar*)malloc((str.length() + 1) * sizeof(hchar));
	memcpy(m_Data, str.c_str(), m_Length * sizeof(hchar));
	m_Length = str.length();
	m_Data[m_Length] = 0;


	if (freeData)
	{
		free(freeData);
	}

	return *this;
}

HazeString& HazeString::operator+(const hchar* str)
{
	if (str)
	{
		decltype(m_Data) freeData = m_Data;

		auto appendLength = std::char_traits<hchar>::length(str);
		m_Data = (hchar*)malloc((m_Length + appendLength + 1) * sizeof(hchar));
		memcpy(m_Data, freeData, m_Length * sizeof(hchar));
		memcpy(m_Data + m_Length, str, appendLength * sizeof(hchar));

		m_Length += appendLength;
		m_Data[m_Length] = 0;


		if (freeData)
		{
			free(freeData);
		}
	}

	return *this;
}

void HazeString::clear()
{
	if (m_Data)
	{
		free(m_Data);
	}

	m_Data = nullptr;
	m_Length = 0;
	m_Hash = 0;
}

//HazeString& HazeString::insert(ui64 index, const hchar* str)
//{
//	return *this;
//}

HazeString operator+(const HazeString::hchar* _Left, const HazeString&& _Right)
{
	HazeString str(_Left);
	return Move(str + _Right);
}

HazeString operator+(const HazeString::hchar* _Left, const HazeString& _Right)
{
	HazeString str(_Left);
	return Move(str + _Right);
}