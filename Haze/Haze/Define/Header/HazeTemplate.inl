
template <typename T>
unsigned int GetSizeByType(HazeDefineType Type, T* This)
{
	return Type.PrimaryType == HazeValueType::Class ? This->GetClassSize(Type.CustomName) : GetSizeByHazeType(Type.PrimaryType);
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

