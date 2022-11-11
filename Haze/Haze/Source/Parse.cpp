#include "Parse.h"

#include <cctype>

#define TOKEN_BOOL L"布尔"
#define TOKEN_CHAR L"字符"
#define TOKEN_BYTE L"字节"
#define TOKEN_SHORT L"双字节"
#define TOKEN_INT L"整数"
#define TOKEN_FLOAT L"小数"
#define TOKEN_LONG L"长整数"
#define TOKEN_DOUBLE L"长小数"

#define TOKEN_UNSIGN_BYTE L"正字节"
#define TOKEN_UNSIGN_SHORT L"正双字节"
#define TOKEN_UNSIGN_INT L"正整数"
#define TOKEN_UNSIGN_FLOAT L"正小数"
#define TOKEN_UNSIGN_LONG L"正长整数"
#define TOKEN_UNSIGN_DOUBLE L"正长小数"

#define TOKEN_CLASS L"类"
#define TOKEN_DATA L"数据"
#define TOKEN_FUNCTION L"函数"

Parse::Parse()
{
	CodeText = nullptr;
}

Parse::~Parse()
{
}

void Parse::InitializeFile()
{
}

void Parse::InitializeString(const std::wstring& String)
{
	CodeText = String.c_str();
}

HazeToken Parse::GetToken()
{
	if (!*CodeText)
	{
		return HazeToken::None;
	}

	while (isspace(*CodeText))
	{
		CodeText++;
	}

	//Match Token
	TokenString = *CodeText;
	while (!isspace(*CodeText))
	{
		TokenString += ++CodeText;
	}

	if (TokenString == TOKEN_BOOL)
	{
		return HazeToken::Bool;
	}
}