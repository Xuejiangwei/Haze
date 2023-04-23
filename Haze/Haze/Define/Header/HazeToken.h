#pragma once

enum class HazeToken : unsigned int
{
	None,

	Identifier,
	
	Void,
	
	Bool,

	Char,

	Int,
	Float,
	Long,
	Double,

	UnsignedInt,
	UnsignedLong,

	StringStart,
	StringEnd,

	Function,

	MainFunction,

	Class,
	ClassData,
	ClassPublic,
	ClassPrivate,
	ClassProtected,

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

	BitAnd,
	BitOr,
	BitXor,

	Shl,
	Shr,

	Assign,
	Equal,
	NotEqual,
	Greater,
	GreaterEqual,
	Less,
	LessEqual,

	Inc,
	Dec,

	AddAssign,
	SubAssign,
	MulAssign,
	DivAssign,
	ModAssign,
	
	BitAndAssign,
	BitOrAssign,
	BitXorAssign,
	
	ShlAssign,
	ShrAssign,

	LeftParentheses,
	RightParentheses,

	Comma,

	LeftBrace,
	RightBrace,

	LeftBrackets,
	RigthBrackets,

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

	New,

	CustomClass,

	//NoMatch
	Number,
	String,

	PointerBase,
	PointerClass,

};