#pragma once

#include <string>

#include "Haze.h"

class OpCodeGenerator;
class ASTBase;

enum class HazeToken
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

	UnsignByte,
	UnsignShort,
	UnsignInt,
	UnsignLong,

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
	void HandleParseExpression();

	std::unique_ptr<ASTBase> ParseExpression();
	std::unique_ptr<ASTBase> ParseUnaryExpression();
	std::unique_ptr<ASTBase> ParseBinaryOperateExpression();

	std::unique_ptr<ASTBase> ParsePrimary();

	std::unique_ptr<ASTBase> ParseVariableDefine();

	std::unique_ptr<ASTBase> ParseBoolExpression();

private:
	bool ExpectNextTokenIs(HazeToken Token, const wchar_t* ErrorInfo);

private:
	class HazeVM* VM;

	std::wstring CodeText;

	HazeToken CurrToken;
	std::wstring CurrLexeme;

	HazeSectionSignal CurrSectionSignal;

	std::shared_ptr<OpCodeGenerator> Generator;
};
