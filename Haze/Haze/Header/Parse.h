#pragma once

#include <string>

enum class HazeToken
{
	None,
	Bool,
	Char,
	Byte,
	Short,
	Int,
	Float,
	Long,
	Double,

	UnsignByte,
	UnsignShort,
	UnsignInt,
	UnsignFloat,
	UnsignLong,
	UnsignDouble,

	Class,
	ClassData,
	ClassFunction,
};

class Parse
{
public:
	Parse();
	~Parse();

	void InitializeFile();

	void InitializeString(const std::wstring& String);

	HazeToken GetToken();

private:
	const wchar_t* CodeText;
	std::wstring TokenString;
};
