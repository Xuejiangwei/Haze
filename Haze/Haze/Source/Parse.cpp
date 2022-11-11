#include "Parse.h"

#include <cctype>

#define TOKEN_BOOL L"����"
#define TOKEN_CHAR L"�ַ�"
#define TOKEN_BYTE L"�ֽ�"
#define TOKEN_SHORT L"˫�ֽ�"
#define TOKEN_INT L"����"
#define TOKEN_FLOAT L"С��"
#define TOKEN_LONG L"������"
#define TOKEN_DOUBLE L"��С��"

#define TOKEN_UNSIGN_BYTE L"���ֽ�"
#define TOKEN_UNSIGN_SHORT L"��˫�ֽ�"
#define TOKEN_UNSIGN_INT L"������"
#define TOKEN_UNSIGN_FLOAT L"��С��"
#define TOKEN_UNSIGN_LONG L"��������"
#define TOKEN_UNSIGN_DOUBLE L"����С��"

#define TOKEN_CLASS L"��"
#define TOKEN_DATA L"����"
#define TOKEN_FUNCTION L"����"

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