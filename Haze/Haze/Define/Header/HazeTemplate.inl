template <typename T>
T StringToInt(const HAZE_STRING& String)
{
	HAZE_STRING_STREAM WSS;
	WSS << String;

	T Ret;
	WSS >> Ret;

	return Ret;
}