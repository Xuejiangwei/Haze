#pragma once

class HazeString
{
public:
	HazeString(const x_HChar* data);

	~HazeString();

	// 禁止拷贝，只允许移动或引用
	HazeString(const HazeString&) = delete;
	HazeString& operator=(const HazeString&) = delete;

	const x_HChar* Data() const { return m_Data; }

	x_uint64 Length() const { return m_Length; }
	
    bool Empty() const { return m_Length == 0; }

    operator HStringView() const
    {
        return HStringView(m_Data, m_Length);
    }

    x_uint64 hash()
    {
        if (m_Hash == 0 && m_Length > 0)
        {
            m_Hash = std::hash<HStringView>{}(HStringView(m_Data, m_Length));
        }

        return m_Hash;
    }

    bool operator==(const HazeString& other) const
    {
        return m_Length == other.m_Length && std::memcmp(m_Data, other.m_Data, m_Length) == 0;
    }

    bool operator==(HStringView sv) const
    {
        return m_Length == sv.length() && std::memcmp(m_Data, sv.data(), m_Length) == 0;
    }

private:
	x_HChar* m_Data;
	x_uint64 m_Length;
    x_uint64 m_Hash;
};
