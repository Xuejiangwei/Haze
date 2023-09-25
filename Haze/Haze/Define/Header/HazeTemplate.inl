#include "HazeUtility.h"

template <typename T>
unsigned int GetSizeByType(HazeDefineType m_Type, T* This)
{
	return m_Type.PrimaryType == HazeValueType::Class ? This->GetClassSize(m_Type.CustomName) :
		m_Type.PrimaryType == HazeValueType::Array ? m_Type.SecondaryType == HazeValueType::Class ? This->GetClassSize(m_Type.CustomName) : GetSizeByHazeType(m_Type.SecondaryType) :
		GetSizeByHazeType(m_Type.PrimaryType);
}

template <typename T>
T StringToStandardType(const HAZE_STRING& String)
{
	HAZE_STRING_STREAM WSS;
	WSS << String;

	T Ret;
	WSS >> Ret;

	return Ret;
}

template<typename T>
inline T StringToStandardType(const HAZE_CHAR* String)
{
	std::stringstream SS;
	SS << String;

	T Ret;
	SS >> Ret;

	return Ret;
}

template <typename T>
T StringToStandardType(const std::string& String)
{
	std::stringstream SS;
	SS << String;

	T Ret;
	SS >> Ret;

	return Ret;
}

template <typename T>
HAZE_BINARY_STRING ToString(T Value)
{
	return HAZE_TO_STR(Value);
}

template <typename T>
HAZE_STRING ToHazeString(T Value)
{
	return HAZE_TO_HAZE_STR(Value);
}
