#pragma once

enum class HazeToken : unsigned int
{
	None,

	Identifier,

	Void,

	Bool,

	Int8,
	UInt8,
	Int16,
	UInt16,
	Int32,
	UInt32,
	Int64,
	UInt64,

	Float32,
	Float64,

	Union,

	Array,
	ArrayDefineEnd,

	StringMatch,

	Function,

	Enum,

	TypeName,
	Template,

	Class,
	ClassData,
	ClassPublic,
	ClassPrivate,

	This,
	ClassAttr,
	
	Deduction,

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

	Break,
	Continue,
	Return,

	While,

	Cast,

	VirtualFunction,
	PureVirtualFunction,

	Define,

	StaticLibrary,
	DLLLibrary,
	ImportModule,

	MultiVariable,

	New,

	CustomClass,
	CustomEnum,

	//NoMatch
	Number,
	String,

	GetAddress,			//ռλ, ��õ�ַ

	ThreeOperatorStart,
	Colon,

	NullPtr,

	SizeOf,

	TwoColon,

	Reference,
};