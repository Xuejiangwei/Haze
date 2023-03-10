#pragma once

enum class HazeToken : unsigned int
{
	None,
	Identifier,
	Void,
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

	Function,

	MainFunction,

	Class,
	ClassData,

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

	Inc,
	Dec,

	LeftParentheses,
	RightParentheses,

	Comma,

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

	StandardLibrary,
	ImportModule,

	MultiVariable,

	//NoMatch
	Number,
	String,
};