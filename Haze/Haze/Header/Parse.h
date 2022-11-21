#pragma once

#include <string>

#include "Haze.h"

class OpCodeGenerator;
class ASTBase;

enum class HazeToken : unsigned int
{
	None,
	Identifier,
	Bool,
	Char,
	Byte,
	Short,
	Int,
	Float,
	Long,
	Double,

	UnsignedByte,
	UnsignedShort,
	UnsignedInt,
	UnsignedLong,

	Class,
	ClassData,
	ClassFunction,

	True,
	False,

	Add,
	Sub,
	Mul,
	Div,
	Mod,

	And,
	Or,
	Not,

	LeftMove,
	RightMove,

	Assign,
	Equal,
	NotEqual,
	Greater,
	GreaterEqual,
	Less,
	LessEqual,

	LeftParentheses,
	RightParentheses,

	LeftBrace,
	RightBrace,

	If,
	Else,

	For,
	ForStep,

	Break,
	Continue,
	Return,

	While,

	Cast,

	Reference,

	Define,

	ImportModule,

	//NoMatch
	Number,
	String,
};

class Parse
{
public:
	Parse(class HazeVM* VM);
	~Parse();

	void InitializeFile(const std::wstring& FilePath);

	void InitializeString(const std::wstring& String);

	void ParseContent();

	HazeToken GetNextToken();

	const std::wstring& GetCurrLexeme() const { return CurrLexeme; }

	const std::wstring& GetLookAtAheadChar() {}

	static bool TokenIsNone(HazeToken Token) { return Token == HazeToken::None; }

private:
	std::unique_ptr<ASTBase> HandleParseExpression();

	std::unique_ptr<ASTBase> ParseExpression();
	std::unique_ptr<ASTBase> ParseUnaryExpression();
	std::unique_ptr<ASTBase> ParseBinaryOperateExpression(std::unique_ptr<ASTBase> Left);

	std::unique_ptr<ASTBase> ParsePrimary();

	std::unique_ptr<ASTBase> ParseVariableDefine();

	std::unique_ptr<ASTBase> ParseBoolExpression();

	std::unique_ptr<ASTBase> ParseNumberExpression();

private:
	bool ExpectNextTokenIs(HazeToken Token, const wchar_t* ErrorInfo);

private:
	class HazeVM* VM;

	HAZE_STRING CodeText;

	HazeToken CurrToken;
	const HAZE_CHAR* CurrCode;
	HAZE_STRING CurrLexeme;

	HazeSectionSignal CurrSectionSignal;

	std::shared_ptr<OpCodeGenerator> Generator;

	HazeValueType VariableDefineType;
};
