
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