#pragma once

class HazeString
{
public:
    using ui64 = unsigned long long;
    using hchar = wchar_t;
    using StrView = std::wstring_view;
    using StrStream = std::wstringstream;
    using STD_Str = std::wstring;

public:
	HazeString(const hchar* data = nullptr);

    HazeString(HazeString&& str);

    HazeString(const STD_Str& str);
	
    ~HazeString();

	// 禁止拷贝，只允许移动或引用
	HazeString(const HazeString&) = delete;
	HazeString& operator=(const HazeString&) = delete;

    HazeString& operator=(HazeString&& str)
    {
        memcpy(this, &str, sizeof(str));
        str.m_Data = nullptr;
    }

    HazeString& operator=(const STD_Str& str);
    
    const hchar* data() const { return m_Data; }

    const hchar* Data() const { return data(); }

    const hchar* c_str() const { return data(); }

    ui64 length() const { return m_Length; }

    ui64 Length() const { return length(); }
    
    ui64 size() const { return length(); }
	
    bool empty() const { return m_Length == 0; }

    bool Empty() const { return empty(); }

    void clear();

    //HazeString& insert(ui64 index, const hchar* str);

    operator StrView() const
    {
        return StrView(m_Data, m_Length);
    }

    ui64 hash() const
    {
        if (m_Hash == 0 && m_Length > 0)
        {
            const_cast<HazeString*>(this)->m_Hash = std::hash<StrView>{}(StrView(m_Data, m_Length));
        }

        return m_Hash;
    }

    ui64 Hash() const { return hash(); }

    bool operator==(const HazeString& other) const
    {
        return m_Length == other.m_Length && std::memcmp(m_Data, other.m_Data, m_Length) == 0;
    }

    bool operator==(StrView sv) const
    {
        return m_Length == sv.length() && std::memcmp(m_Data, sv.data(), m_Length) == 0;
    }

    bool operator==(const hchar* str) const
    {
        return m_Length == std::char_traits<hchar>::length(str) && std::memcmp(m_Data, str, m_Length) == 0;
    }

    HazeString& operator+(const hchar* str);

    HazeString& operator+(const HazeString& str)
    {
        return HazeString::operator+(str.c_str());
    }

    HazeString& operator+(const STD_Str& str)
    {
        return HazeString::operator+(str.c_str());
    }

    friend StrStream& operator<<(StrStream& stream, const HazeString& str)
    {
        stream << str.m_Data;
        return stream;
    }

    hchar& operator[](ui64 off)
    {
        return *(this->m_Data + off);
    }

    constexpr hchar& operator[](ui64 off) const noexcept
    {
        return *(this->m_Data + off);
    }

private:
	hchar* m_Data;
    ui64 m_Length;
    ui64 m_Hash;
};

HazeString operator+(const HazeString::hchar* _Left, const HazeString&& _Right);
HazeString operator+(const HazeString::hchar* _Left, const HazeString& _Right);
