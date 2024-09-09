#include "HazeUtility.h"

template <typename T>
T StringToStandardType(const HString& str)
{
	HAZE_STRING_STREAM wss;
	wss << str;

	T ret;
	wss >> ret;

	return ret;
}

template<typename T>
inline T StringToStandardType(const HChar* str)
{
	std::stringstream ss;
	ss << str;

	T ret;
	ss >> ret;

	return ret;
}

template <typename T>
T StringToStandardType(const std::string& str)
{
	std::stringstream ss;
	ss << str;

	T ret;
	ss >> ret;

	return ret;
}

template <typename T>
HAZE_BINARY_STRING ToString(T value)
{
	return HAZE_TO_STR(value);
}

template <typename T>
HString ToHazeString(T value)
{
	return HAZE_TO_HAZE_STR(value);
}