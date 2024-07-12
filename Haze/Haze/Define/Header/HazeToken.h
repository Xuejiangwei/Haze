#pragma once

enum class HazeToken : unsigned int
{
	None,

	Identifier,

	Void,

	Bool,

	Byte,

	Char,

	Short,
	Int,
	Float,
	Long,
	Double,

	UnsignedByte,
	UnsignedShort,
	UnsignedInt,
	UnsignedLong,

	V_Array,
	ArrayDefineEnd,
	ArrayLength,

	StringMatch,

	Function,

	Enum,

	TypeName,
	Template,

	Class,
	m_ClassDatas,
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

	VirtualFunction,
	PureVirtualFunction,

	ReferenceBase,
	ReferenceClass,

	Define,

	StandardLibrary,
	DLLLibrary,
	ImportModule,

	MultiVariable,

	New,

	CustomClass,
	CustomEnum,

	//NoMatch
	Number,
	String,

	PointerBase,
	PointerClass,
	PointerFunction,
	PointerPointer,

	PointerValue,		//ռλ, ��ָ��
	GetAddress,			//ռλ, ��õ�ַ

	ThreeOperatorStart,
	Colon,

	NullPtr,

	SizeOf,

	TwoColon,
};