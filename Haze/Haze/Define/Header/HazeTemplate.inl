#include "HazeUtility.h"

template <typename T>
unsigned int GetSizeByType(HazeDefineType Type, T* This)
{
	return Type.PrimaryType == HazeValueType::Class ? This->GetClassSize(Type.CustomName) :
		Type.PrimaryType == HazeValueType::Array ? Type.SecondaryType == HazeValueType::Class ? This->GetClassSize(Type.CustomName) : GetSizeByHazeType(Type.SecondaryType) :
		GetSizeByHazeType(Type.PrimaryType);
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
HAZE_BINARY_STRING ToString(T m_Value)
{
	return HAZE_TO_STR(m_Value);
}

template <typename T>
HAZE_STRING ToHazeString(T m_Value)
{
	return HAZE_TO_HAZE_STR(m_Value);
}
