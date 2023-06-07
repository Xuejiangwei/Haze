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
HAZE_BINARY_STRING ToString(T Value)
{
	return HAZE_TO_STR(Value);
}

template <typename T>
HAZE_STRING ToHazeString(T Value)
{
	return HAZE_TO_HAZE_STR(Value);
}

template<typename T>
inline T GetHazeValueByBaseType(const char* Address, HazeValueType Type)
{
	switch (Type)
	{
	case HazeValueType::Bool:
	{
		bool Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::Byte:
	{
		hbyte Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::UnsignedByte:
	{
		uhbyte Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::Char:
	{
		char Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::Short:
	{
		short Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::UnsignedShort:
	{
		ushort Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::Int:
	{
		int Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::Float:
	{
		float Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::Long:
	{
		int64 Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::Double:
	{
		double Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::UnsignedInt:
	{
		uint32 Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::UnsignedLong:
	{
		uint64 Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
		break;
	case HazeValueType::Array:
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
	case HazeValueType::PointerFunction:
	case HazeValueType::PointerArray:
	case HazeValueType::PointerPointer:
	case HazeValueType::ReferenceBase:
	case HazeValueType::ReferenceClass:
	{
		uint64 Value;
		memcpy(&Value, Address, sizeof(Value));
		return Value;
	}
	default:
		break;
	}

	return 0;
}
