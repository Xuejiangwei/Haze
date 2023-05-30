#pragma once

enum class HazeToken : unsigned int
{
	None,

	Identifier,
	
	Void,
	
	Bool,

	Byte,

	Char,

	Int,
	Float,
	Long,
	Double,

	UnsignedInt,
	UnsignedLong,

	Array,

	StringMatch,

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
	BitNeg,
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

	ReferenceBase,
	ReferenceClass,

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
	PointerFunction,
	PointerPointer,

	PointerValue,		//占位, 解指针
	GetAddress,			//占位, 获得地址

	ThreeOperatorStart,
	Colon,

	NullPtr,
};