﻿#include <cctype>
#include <fstream>

#include "Haze.h"
#include "HazeVM.h"
#include "Parse.h"
#include "ASTBase.h"
#include "ASTFunction.h"
#include "ASTClass.h"
#include "ASTStandardLibrary.h"

#include "HazeCompiler.h"

#include "HazeLog.h"

#define HAZE_SINGLE_COMMENT				HAZE_TEXT("#")
#define HAZE_MULTI_COMMENT_START		HAZE_TEXT("/*")
#define HAZE_MULTI_COMMENT_END			HAZE_TEXT("*/")

#define TOKEN_VOID						HAZE_TEXT("空")
#define TOKEN_BOOL						HAZE_TEXT("布尔")
#define TOKEN_SHORT						HAZE_TEXT("双字节")
#define TOKEN_INT						HAZE_TEXT("整数")
#define TOKEN_FLOAT						HAZE_TEXT("小数")
#define TOKEN_LONG						HAZE_TEXT("长整数")
#define TOKEN_DOUBLE					HAZE_TEXT("长小数")

#define TOKEN_UNSIGNED_BYTE				HAZE_TEXT("正字节")
#define TOKEN_UNSIGNED_SHORT			HAZE_TEXT("正双字节")
#define TOKEN_UNSIGNED_INT				HAZE_TEXT("正整数")
#define TOKEN_UNSIGNED_LONG				HAZE_TEXT("正长整数")

#define TOKEN_STRING					HAZE_TEXT("字符")
#define TOKEN_STRING_START				HAZE_TEXT("[")
#define TOKEN_STRING_END				HAZE_TEXT("]")

#define TOKEN_CLASS						HAZE_TEXT("类")
#define TOKEN_CLASS_DATA				HAZE_TEXT("数据")
#define TOKEN_CLASS_DATA_PUBLIC			HAZE_TEXT("公")
#define TOKEN_CLASS_DATA_PRIVATE		HAZE_TEXT("私")
#define TOKEN_CLASS_DATA_PROTECTED		HAZE_TEXT("共")

#define TOKEN_FUNCTION					HAZE_TEXT("函数")

#define TOKEN_MAIN_FUNCTION				HAZE_MAIN_FUNCTION_TEXT

#define TOKEN_TRUE						HAZE_TEXT("真")
#define TOKEN_FALSE						HAZE_TEXT("假")

#define TOKEN_ADD						HAZE_TEXT("+")
#define TOKEN_SUB						HAZE_TEXT("-")
#define TOKEN_MUL						HAZE_TEXT("*")
#define TOKEN_DIV						HAZE_TEXT("/")
#define TOKEN_MOD						HAZE_TEXT("%")

#define TOKEN_AND						HAZE_TEXT("与")
#define TOKEN_OR						HAZE_TEXT("或")
#define TOKEN_NOT						HAZE_TEXT("非")

#define TOKEN_LEFT_MOVE					HAZE_TEXT("<<")
#define TOKEN_RIGHT_MOVE				HAZE_TEXT(">>")

#define TOKEN_ASSIGN					HAZE_TEXT("=")
#define TOKEN_EQUAL						HAZE_TEXT("==")
#define TOKEN_NOT_EQUAL					HAZE_TEXT("!=")
#define TOKEN_GREATER					HAZE_TEXT(">")
#define TOKEN_GREATER_EQUAL				HAZE_TEXT(">=")
#define TOKEN_LESS						HAZE_TEXT("<")
#define TOKEN_LESS_EQUAL				HAZE_TEXT("<=")

#define TOKEN_INC						HAZE_TEXT("++")
#define TOKEN_DEC						HAZE_TEXT("--")

#define TOKEN_LEFT_PARENTHESES			HAZE_TEXT("(")
#define TOKEN_RIGHT_PARENTHESES			HAZE_TEXT(")")

#define TOKEN_COMMA						HAZE_TEXT(",")

#define TOKEN_LEFT_BRACE				HAZE_TEXT("{")
#define TOKEN_RIGHT_BRACE				HAZE_TEXT("}")

//
#define TOKEN_IF						HAZE_TEXT("若")
#define TOKEN_ELSE						HAZE_TEXT("否则")

#define TOKEN_FOR						HAZE_TEXT("遍历")
#define TOKEN_FOR_STEP					HAZE_TEXT("步进")

#define TOKEN_BREAK						HAZE_TEXT("打断")
#define TOKEN_CONTINUE					HAZE_TEXT("继续")
#define TOKEN_RETURN					HAZE_TEXT("返回")

#define TOKEN_WHILE						HAZE_TEXT("当")

#define TOKEN_CAST						HAZE_TEXT("转")

#define TOKEN_REFERENCE					HAZE_TEXT("引用")

#define TOKEN_DEFINE					HAZE_TEXT("定义")

#define TOKEN_STANDARD_LIBRARY			HAZE_TEXT("标准库")
#define TOKEN_IMPORT_MODULE				HAZE_TEXT("引")

#define TOKEN_MULTI_VARIABLE			HAZE_TEXT("...")

#define TOKEN_NEW						HAZE_TEXT("得")

static std::unordered_map<HAZE_STRING, HazeToken> MapToken =
{
	{TOKEN_VOID, HazeToken::Void},
	{TOKEN_BOOL, HazeToken::Bool},
	{TOKEN_SHORT, HazeToken::Short},
	{TOKEN_INT, HazeToken::Int},
	{TOKEN_FLOAT, HazeToken::Float},
	{TOKEN_LONG, HazeToken::Long},
	{TOKEN_DOUBLE, HazeToken::Double},

	{TOKEN_UNSIGNED_SHORT, HazeToken::UnsignedShort},
	{TOKEN_UNSIGNED_INT, HazeToken::UnsignedInt},
	{TOKEN_UNSIGNED_LONG, HazeToken::UnsignedLong},

	{TOKEN_STRING, HazeToken::String},

	{TOKEN_STRING_START, HazeToken::StringStart},
	{TOKEN_STRING_END, HazeToken::StringEnd},

	{TOKEN_FUNCTION, HazeToken::Function},

	{TOKEN_MAIN_FUNCTION, HazeToken::MainFunction},

	{TOKEN_CLASS, HazeToken::Class},
	{TOKEN_CLASS_DATA, HazeToken::ClassData},
	{TOKEN_CLASS_DATA_PUBLIC, HazeToken::ClassPublic},
	{TOKEN_CLASS_DATA_PRIVATE, HazeToken::ClassPrivate},
	{TOKEN_CLASS_DATA_PROTECTED, HazeToken::ClassProtected},

	{TOKEN_TRUE, HazeToken::True},
	{TOKEN_FALSE, HazeToken::False},

	{TOKEN_ADD, HazeToken::Add},
	{TOKEN_SUB, HazeToken::Sub},
	{TOKEN_MUL, HazeToken::Mul},
	{TOKEN_DIV, HazeToken::Div},
	{TOKEN_MOD, HazeToken::Mod},

	{TOKEN_AND, HazeToken::And},
	{TOKEN_OR, HazeToken::Or},
	{TOKEN_NOT, HazeToken::Not},

	{TOKEN_LEFT_MOVE, HazeToken::LeftMove},
	{TOKEN_NOT, HazeToken::RightMove},

	{TOKEN_ASSIGN, HazeToken::Assign},
	{TOKEN_EQUAL, HazeToken::Equal},
	{TOKEN_NOT_EQUAL, HazeToken::NotEqual},
	{TOKEN_GREATER, HazeToken::Greater},
	{TOKEN_GREATER_EQUAL, HazeToken::GreaterEqual},
	{TOKEN_LESS, HazeToken::Less},
	{TOKEN_LESS_EQUAL, HazeToken::LessEqual},

	{TOKEN_INC, HazeToken::Inc},
	{TOKEN_DEC, HazeToken::Dec},

	{TOKEN_LEFT_PARENTHESES, HazeToken::LeftParentheses},
	{TOKEN_RIGHT_PARENTHESES, HazeToken::RightParentheses},

	{TOKEN_COMMA, HazeToken::Comma},

	{TOKEN_LEFT_BRACE, HazeToken::LeftBrace},
	{TOKEN_RIGHT_BRACE, HazeToken::RightBrace},

	{TOKEN_IF, HazeToken::If},
	{TOKEN_ELSE, HazeToken::Else},

	{TOKEN_FOR, HazeToken::For},
	{TOKEN_FOR_STEP, HazeToken::ForStep},

	{TOKEN_BREAK, HazeToken::Break},
	{TOKEN_CONTINUE, HazeToken::Continue},
	{TOKEN_RETURN, HazeToken::Return},

	{TOKEN_WHILE, HazeToken::While},

	{TOKEN_CAST, HazeToken::Cast},

	{TOKEN_REFERENCE, HazeToken::Reference},

	{TOKEN_DEFINE, HazeToken::Define},

	{TOKEN_STANDARD_LIBRARY, HazeToken::StandardLibrary},
	{TOKEN_IMPORT_MODULE, HazeToken::ImportModule},

	{TOKEN_MULTI_VARIABLE, HazeToken::MultiVariable},

	{TOKEN_NEW, HazeToken::New},
};

static std::unordered_map<HazeToken, int> MapBinOp =
{
	{ HazeToken::Assign, 10 },
	{ HazeToken::Or, 14 },
	{ HazeToken::And, 15 },

	{ HazeToken::Equal, 20 },
	{ HazeToken::NotEqual, 20 },

	{ HazeToken::Greater, 21 },
	{ HazeToken::GreaterEqual, 21 },
	{ HazeToken::Less, 21 },
	{ HazeToken::LessEqual, 21 },

	{ HazeToken::LeftMove, 30 },
	{ HazeToken::RightMove, 30 },


	{ HazeToken::Add, 40 },
	{ HazeToken::Sub, 40 },

	{ HazeToken::Mul, 50 },
	{ HazeToken::Div, 50 },
	{ HazeToken::Mod, 50 },

	{ HazeToken::Not, 60 },
	{ HazeToken::Inc, 60 },
};

//static std::unordered_map<HazeToken, HazeValueType> MapValueType =
//{
//	{HazeToken::Bool, HazeValueType::Bool},
//	{HazeToken::Char, HazeValueType::Char},
//	{HazeToken::Byte, HazeValueType::Byte},
//	{HazeToken::Short, HazeValueType::Short},
//	{HazeToken::Int, HazeValueType::Int},
//	{HazeToken::Float, HazeValueType::Float},
//	{HazeToken::Long, HazeValueType::Long},
//	{HazeToken::Double, HazeValueType::Double},
//	{HazeToken::UnsignedByte, HazeValueType::UnsignedByte},
//	{HazeToken::UnsignedShort, HazeValueType::UnsignedShort},
//	{HazeToken::UnsignedInt, HazeValueType::UnsignedInt},
//	{HazeToken::UnsignedLong, HazeValueType::UnsignedLong},
//};

Parse::Parse(HazeVM* VM) :VM(VM)
{
}

Parse::~Parse()
{
}

void Parse::InitializeFile(const HAZE_STRING& FilePath)
{
	HAZE_IFSTREAM FS(FilePath);
	FS.imbue(std::locale("chs"));
	HAZE_STRING Content(std::istreambuf_iterator<HAZE_CHAR>(FS), {});
	CodeText = std::move(Content);
	CurrCode = CodeText.c_str();
	FS.close();
}

void Parse::InitializeString(const HAZE_STRING& String)
{
	CodeText = String;
}

void Parse::ParseContent()
{
	StackSectionSignal.push(HazeSectionSignal::Global);
	GetNextToken();

	while (!TokenIsNone(CurrToken))
	{
		switch (CurrToken)
		{
		case HazeToken::Bool:
		case HazeToken::Short:
		case HazeToken::Int:
		case HazeToken::Float:
		case HazeToken::Long:
		case HazeToken::Double:
		case HazeToken::UnsignedShort:
		case HazeToken::UnsignedInt:
		case HazeToken::UnsignedLong:
		case HazeToken::String:
		case HazeToken::Identifier:
		{
			auto AST = ParseExpression();
			AST->CodeGen();
		}
		break;
		case HazeToken::Class:
		{
			auto AST = ParseClass();
			AST->CodeGen();
		}
			break;
		case HazeToken::Function:
		{
			auto AST = ParseFunctionSection();
			AST->CodeGen();
		}
		break;
		case HazeToken::MainFunction:
		{
			auto AST = ParseMainFunction();
			AST->CodeGen();
		}
		break;
		case HazeToken::StandardLibrary:
		{
			auto AST = ParseStandardLibrary();
			AST->CodeGen();
		}
		break;
		case HazeToken::ImportModule:
		{
			auto AST = ParseImportModule();
			AST->CodeGen();
		}
		break;
		default:
			break;
		}
	}

	StackSectionSignal.pop();
}

HazeToken Parse::GetNextToken()
{
	if (!*CurrCode)
	{
		CurrToken = HazeToken::None;
		CurrLexeme = HAZE_TEXT("");
		return HazeToken::None;
	}

	while (isspace(*CurrCode))
	{
		CurrCode++;
	}

	if (HAZE_STRING(CurrCode) == HAZE_TEXT(""))
	{
		return HazeToken::None;
	}

	//Match Token
	CurrLexeme.clear();
	while (!isspace(*CurrCode))
	{
		if (IsHazeSignalToken(*CurrCode))
		{
			if (CurrLexeme.length() == 0)
			{
				CurrLexeme += *(CurrCode++);
			}
			break;
		}

		CurrLexeme += *(CurrCode++);
	}

	if (CurrLexeme == HAZE_STRING(HAZE_SINGLE_COMMENT))
	{
		HAZE_STRING WS;
		WS = *CurrCode;
		while (WS != HAZE_TEXT("\n"))
		{
			CurrCode++;
			WS = *CurrCode;
		}
		return GetNextToken();
	}

	auto It = MapToken.find(CurrLexeme);
	if (It != MapToken.end())
	{
		CurrToken = It->second;
	}
	else if (IsNumber(CurrLexeme))
	{
		CurrToken = HazeToken::Number;
	}
	else if (IsPointer(CurrLexeme))
	{
		CurrToken = HazeToken::Pointer;
	}
	else
	{
		CurrToken = HazeToken::Identifier;
	}

	return CurrToken;
}

std::unique_ptr<ASTBase> Parse::HandleParseExpression()
{
	return ParseExpression();
}

std::unique_ptr<ASTBase> Parse::ParseExpression()
{
	std::unique_ptr<ASTBase> Left = ParseUnaryExpression();

	if (Left)
	{
		return ParseBinaryOperateExpression(0, std::move(Left));
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseUnaryExpression()
{
	auto it = MapBinOp.find(CurrToken);
	if (it == MapBinOp.end())
	{
		return ParsePrimary();
	}
	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseBinaryOperateExpression(int Prec, std::unique_ptr<ASTBase> Left)
{
	// highest.

	auto it = MapBinOp.find(CurrToken);
	if (it == MapBinOp.end())
	{
		return Left;
	}
	else
	{
		while (true)
		{
			if (it->second < Prec)
			{
				return Left;
			}

			HazeToken OpToken = CurrToken;

			GetNextToken();

			std::unique_ptr<ASTBase> Right = ParseUnaryExpression();
			if (!Right)
			{
				return nullptr;
			}

			auto NextPrec = MapBinOp.find(CurrToken);
			if (NextPrec != MapBinOp.end() && it->second < NextPrec->second)
			{
				Right = ParseBinaryOperateExpression(it->second, std::move(Right));
				if (!Right)
				{
					return nullptr;
				}

				NextPrec = MapBinOp.find(CurrToken);
				if (NextPrec == MapBinOp.end())
				{
					return std::make_unique<ASTBinaryExpression>(VM, StackSectionSignal.top(), OpToken, Left, Right);
				}
			}
			else
			{
				return std::make_unique<ASTBinaryExpression>(VM, StackSectionSignal.top(), OpToken, Left, Right);
			}

			// Merge LHS/RHS.
			Left = std::make_unique<ASTBinaryExpression>(VM, StackSectionSignal.top(), OpToken, Left, Right);
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParsePrimary()
{
	HazeToken Token = CurrToken;
	switch (Token)
	{
	case HazeToken::Bool:
	case HazeToken::Short:
	case HazeToken::Int:
	case HazeToken::Float:
	case HazeToken::Long:
	case HazeToken::Double:
	case HazeToken::UnsignedShort:
	case HazeToken::UnsignedInt:
	case HazeToken::UnsignedLong:
	case HazeToken::String:
		return ParseVariableDefine();
	case HazeToken::Number:
		return ParseNumberExpression();
	case HazeToken::StringStart:
		return ParseStringText();
	case HazeToken::True:
	case HazeToken::False:
		return ParseBoolExpression();
	case HazeToken::Identifier:
		return ParseIdentifer();
	case HazeToken::Return:
		return ParseReturn();
	case HazeToken::New:
		return ParseNew();
	default:
		break;
	}
	return std::unique_ptr<ASTBase>();
}

std::unique_ptr<ASTBase> Parse::ParseIdentifer()
{
	HAZE_STRING IdentiferName = CurrLexeme;
	if (GetNextToken() == HazeToken::LeftParentheses)
	{
		//函数调用
		std::vector<std::unique_ptr<ASTBase>> VectorParam;
		while (true)
		{
			HazeDefineVariable Param;
			if (GetNextToken() == HazeToken::RightParentheses)
			{
				break;
			}

			VectorParam.push_back(ParseExpression());

			if (!TokenIs(HazeToken::Comma))
			{
				break;
			}
		}

		GetNextToken();
		return std::make_unique<ASTFunctionCall>(VM, StackSectionSignal.top(), IdentiferName, VectorParam);
	}
	else
	{
		return std::make_unique<ASTIdentifier>(VM, StackSectionSignal.top(), IdentiferName);
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseVariableDefine()
{
	DefineVariable.Type.Type = GetValueTypeByToken(CurrToken);
	if (CurrToken == HazeToken::Identifier)
	{
		DefineVariable.Type.Type = HazeValueType::Null;
		DefineVariable.Type.CustomName = CurrLexeme;
	}

	if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("Error: Parse bool expression name wrong\n")))
	{
		DefineVariable.Name = CurrLexeme;

		if (ExpectNextTokenIs(HazeToken::Assign, HAZE_TEXT("Error: Parse bool expression expect = \n")))
		{
			GetNextToken();		//吃掉赋值符号
			std::unique_ptr<ASTBase> Expression = ParseExpression();

			return std::make_unique<ASTVariableDefine>(VM, StackSectionSignal.top(), DefineVariable, std::move(Expression));
		}
		else if ((CurrToken == HazeToken::RightParentheses || CurrToken == HazeToken::Comma) && StackSectionSignal.top() == HazeSectionSignal::Function)
		{
			//函数调用
			return std::make_unique<ASTVariableDefine>(VM, StackSectionSignal.top(), DefineVariable, nullptr);
		}
		else
		{
			return std::make_unique<ASTVariableDefine>(VM, StackSectionSignal.top(), DefineVariable, nullptr);
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseStringText()
{
	GetNextToken();
	HAZE_STRING Text = CurrLexeme;

	if (ExpectNextTokenIs(HazeToken::StringEnd, HAZE_TEXT("Parse string expect end signal ] ")))
	{
		GetNextToken();
		return std::make_unique<ASTStringText>(VM, Text);
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseBoolExpression()
{
	HazeValue Value;
	Value.Type = HazeValueType::Bool;
	Value.Value.Bool = CurrLexeme == TOKEN_TRUE;

	return std::make_unique<ASTBool>(VM, Value);
}

std::unique_ptr<ASTBase> Parse::ParseNumberExpression()
{
	HazeValue Value;
	if (DefineVariable.Type.Type != HazeValueType::Null)
	{
		Value.Type = DefineVariable.Type.Type;
	}
	else
	{
		Value.Type = GetNumberDefaultType(CurrLexeme);
	}

	StringToHazeValueNumber(CurrLexeme, Value);

	GetNextToken();
	return std::make_unique<ASTNumber>(VM, Value);
}

std::unique_ptr<ASTBase> Parse::ParseIfExpression()
{
	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseForExpression()
{
	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseWhileExpression()
{
	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseReturn()
{
	GetNextToken();
	auto ReturnExpression = ParseExpression();
	return std::make_unique<ASTReturn>(VM, ReturnExpression);
}

std::unique_ptr<ASTBase> Parse::ParseNew()
{
	GetNextToken();

	HazeDefineVariable Define;
	Define.Type.Type = GetValueTypeByToken(CurrToken);
	Define.Type.CustomName = CurrLexeme;

	GetNextToken();
	return std::make_unique<ASTNew>(VM, Define);
}

std::unique_ptr<ASTBase> Parse::ParseMultiExpression()
{
	std::vector<std::unique_ptr<ASTBase>> VectorExpr;

	GetNextToken();
	while (auto e = ParseExpression())
	{
		VectorExpr.push_back(std::move(e));

		if (TokenIs(HazeToken::RightBrace))
		{
			break;
		}
	}

	return std::make_unique<ASTMultiExpression>(VM, StackSectionSignal.top(), VectorExpr);
}

std::unique_ptr<ASTFunctionSection> Parse::ParseFunctionSection()
{
	if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("Error: Parse function expression expect { \n")))
	{
		std::vector<std::unique_ptr<ASTFunction>> Functions;

		GetNextToken();
		while (CurrToken != HazeToken::RightBrace)
		{
			Functions.push_back(ParseFunction());
		}

		GetNextToken();
		return std::make_unique<ASTFunctionSection>(VM, Functions);
	}

	return nullptr;
}

std::unique_ptr<ASTFunction> Parse::ParseFunction(const HAZE_STRING* ClassName)
{
	StackSectionSignal.push(HazeSectionSignal::Function);

	//获得函数返回类型及是自定义类型时获得类型名字
	HazeDefineData FuncType;
	FuncType.Type = GetValueTypeByToken(CurrToken);
	if (CurrToken == HazeToken::Identifier)
	{
		FuncType.CustomName = CurrLexeme;
	}

	//获得函数名
	if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("Error: Parse function expression expect correct function name \n")))
	{
		HAZE_STRING FunctionName = CurrLexeme;
		if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("Error: Parse function expression expect function param left need ( \n")))
		{
			std::vector<HazeDefineVariable> Vector_Param;

			if (ClassName && !ClassName->empty())
			{
				HazeDefineVariable ThisParam;
				ThisParam.Name = HAZE_CLASS_THIS;
				ThisParam.Type.Type = HazeValueType::Pointer;
				ThisParam.Type.CustomName = *ClassName;
				
				Vector_Param.push_back(ThisParam);
			}

			while (!TokenIs(HazeToken::LeftBrace))
			{
				HazeDefineVariable Param;
				if (GetNextToken() == HazeToken::RightParentheses)
				{
					break;
				}

				Param.Type.Type = GetValueTypeByToken(CurrToken);
				if (CurrToken == HazeToken::Identifier)
				{
					Param.Type.Type = HazeValueType::Null;
					Param.Type.CustomName = CurrLexeme;
				}

				if (Param.Type.Type == HazeValueType::MultiVar)
				{
					ExpectNextTokenIs(HazeToken::RightParentheses, HAZE_TEXT("Error: Parse function expression expect function mutiply param right need ) \n"));
					Vector_Param.push_back(Param);

					GetNextToken();
					break;
				}

				GetNextToken();
				Param.Name = CurrLexeme;

				Vector_Param.push_back(Param);

				if (!ExpectNextTokenIs(HazeToken::Comma))
				{
					break;
				}
			}

			if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("Error: Parse function body expect { \n")))
			{
				std::unique_ptr<ASTBase> Body = ParseMultiExpression();

				if (TokenIs(HazeToken::RightBrace, HAZE_TEXT("Error: Parse function expression expect function body expect } \n")))
				{
					StackSectionSignal.pop();

					GetNextToken();
					return std::make_unique<ASTFunction>(VM, StackSectionSignal.top(), FunctionName, FuncType, Vector_Param, Body);
				}
			}
		}
	}

	return nullptr;
}

std::unique_ptr<ASTFunction> Parse::ParseMainFunction()
{
	HAZE_STRING FunctionName = CurrLexeme;
	StackSectionSignal.push(HazeSectionSignal::Function);
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("Error: Parse function expression expect function param left need ( \n")))
	{
		std::vector<HazeDefineVariable> Vector_Param;

		while (!TokenIs(HazeToken::LeftBrace))
		{
			HazeDefineVariable Param;
			if (GetNextToken() == HazeToken::RightParentheses)
			{
				break;
			}

			Param.Type.Type = GetValueTypeByToken(CurrToken);
			if (CurrToken == HazeToken::Identifier)
			{
				Param.Type.Type = HazeValueType::Null;
				Param.Type.CustomName = CurrLexeme;
			}

			GetNextToken();
			Param.Name = CurrLexeme;

			Vector_Param.push_back(Param);

			if (!ExpectNextTokenIs(HazeToken::Comma))
			{
				break;
			}
		}

		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("Error: Parse function body expect { \n")))
		{
			std::unique_ptr<ASTBase> Body = ParseMultiExpression();

			if (TokenIs(HazeToken::RightBrace, HAZE_TEXT("Error: Parse function expression expect function body expect } \n")))
			{
				StackSectionSignal.pop();

				GetNextToken();
				HazeDefineData DefineType = { HazeValueType::Int, HAZE_TEXT("") };
				return std::make_unique<ASTFunction>(VM, StackSectionSignal.top(), FunctionName, DefineType, Vector_Param, Body);
			}
		}
	}

	return nullptr;
}

std::unique_ptr<ASTStandardLibrary> Parse::ParseStandardLibrary()
{
	GetNextToken();
	HAZE_STRING StandardLibraryName = CurrLexeme;

	StackSectionSignal.push(HazeSectionSignal::StandardLibrary);

	if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("Error: Parse standard library expect { \n")))
	{
		std::vector<std::unique_ptr<ASTClassDefine>> Vector_ClassDefine;
		std::vector<std::unique_ptr<ASTFunctionDefine>> Vector_FunctionDefine;

		GetNextToken();
		while (CurrToken == HazeToken::Function || CurrToken == HazeToken::Class)
		{
			if (CurrToken == HazeToken::Function)
			{
				auto Vector_Function = ParseStandardLibrary_FunctionDefine();
				for (auto& Iter : Vector_Function)
				{
					Vector_FunctionDefine.push_back(std::move(Iter));
				}
			}
			else if (CurrToken == HazeToken::Class)
			{
				Vector_ClassDefine.push_back(ParseStandardLibrary_ClassDefine());
			}
		}

		StackSectionSignal.pop();
		GetNextToken();

		return std::make_unique<ASTStandardLibrary>(VM, StandardLibraryName, Vector_FunctionDefine, Vector_ClassDefine);
	}

	return nullptr;
}

std::unique_ptr<ASTClassDefine> Parse::ParseStandardLibrary_ClassDefine()
{
	/*std::vector<std::vector<std::unique_ptr<ASTBase>>> Data;
	std::vector<std::unique_ptr<ASTFunctionDefine>> Function;
	return std::make_unique<ASTClassDefine>(VM, CurrLexeme, Data, Function);*/
	return nullptr;
}

std::vector<std::unique_ptr<ASTFunctionDefine>> Parse::ParseStandardLibrary_FunctionDefine()
{
	std::vector<std::unique_ptr<ASTFunctionDefine>> Vector_FunctionDefine;

	if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("Error: Parse standard library function section expect { \n")))
	{
		GetNextToken();
		while (CurrToken != HazeToken::RightBrace)
		{
			StackSectionSignal.push(HazeSectionSignal::Function);

			//获得函数返回类型及是自定义类型时获得类型名字
			HazeDefineData FuncType;
			FuncType.Type = GetValueTypeByToken(CurrToken);
			if (CurrToken == HazeToken::Identifier)
			{
				FuncType.CustomName = CurrLexeme;
			}

			//获得函数名
			if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("Error: Parse function expression expect correct function name \n")))
			{
				HAZE_STRING FunctionName = CurrLexeme;
				if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("Error: Parse function expression expect function param left need ( \n")))
				{
					std::vector<HazeDefineVariable> Vector_Param;

					while (!TokenIs(HazeToken::LeftBrace))
					{
						HazeDefineVariable Param;
						if (GetNextToken() == HazeToken::RightParentheses)
						{
							break;
						}

						Param.Type.Type = GetValueTypeByToken(CurrToken);
						if (CurrToken == HazeToken::Identifier)
						{
							Param.Type.Type = HazeValueType::Null;
							Param.Type.CustomName = CurrLexeme;
						}

						if (Param.Type.Type == HazeValueType::MultiVar)
						{
							ExpectNextTokenIs(HazeToken::RightParentheses, HAZE_TEXT("Error: Parse function expression expect function mutiply param right need ) \n"));
							Vector_Param.push_back(Param);

							GetNextToken();
							break;
						}

						GetNextToken();
						Param.Name = CurrLexeme;

						Vector_Param.push_back(Param);

						if (!ExpectNextTokenIs(HazeToken::Comma))
						{
							break;
						}
					}

					StackSectionSignal.pop();
					Vector_FunctionDefine.push_back(std::make_unique<ASTFunctionDefine>(VM, FunctionName, FuncType, Vector_Param));

				}
			}
		}

		ExpectNextTokenIs(HazeToken::RightBrace, HAZE_TEXT("Error: Parse standard library end need } \n"));
		StackSectionSignal.pop();

		GetNextToken();

		//return std::make_unique<ASTStandardLibrary>(VM, StandardLibraryName, Vector_FunctionDefine);
	}

	return Vector_FunctionDefine;
}

std::unique_ptr<ASTBase> Parse::ParseImportModule()
{
	GetNextToken();
	return std::make_unique<ASTImportModule>(VM, CurrLexeme);
}

std::unique_ptr<ASTClass> Parse::ParseClass()
{
	if (ExpectNextTokenIs(HazeToken::Identifier, (HAZE_TEXT("expect correct class name not ") + CurrLexeme).c_str()))
	{
		HAZE_STRING Name = CurrLexeme;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("expect { ")))
		{
			StackSectionSignal.push(HazeSectionSignal::Class);

			std::vector<std::vector<std::unique_ptr<ASTBase>>> Vector_ClassData;
			std::unique_ptr<ASTFunctionSection> ClassFunction;

			GetNextToken();
			while (CurrToken == HazeToken::ClassData || CurrToken == HazeToken::Function)
			{
				if (CurrToken == HazeToken::ClassData)
				{
					if (Vector_ClassData.size() == 0)
					{
						Vector_ClassData = ParseClassData();
					}
					else
					{
						HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("class only one data define section  class :%ws \n"), Name);
					}
				}
				else if (CurrToken == HazeToken::Function)
				{
					ClassFunction = ParseFunctionSection();
				}
			}

			StackSectionSignal.pop();
			return std::make_unique<ASTClass>(VM, Name, Vector_ClassData, ClassFunction);
		}
	}

	return nullptr;
}

std::vector<std::vector<std::unique_ptr<ASTBase>>> Parse::ParseClassData()
{
	std::vector<std::vector<std::unique_ptr<ASTBase>>> Vector_ClassData;

	if (ExpectNextTokenIs(HazeToken::LeftBrace, (HAZE_TEXT("expect { ") + CurrLexeme).c_str()))
	{
		GetNextToken();

		std::vector<std::unique_ptr<ASTBase>> Vector_ClassDataPublic;
		std::vector<std::unique_ptr<ASTBase>> Vector_ClassDataPrivate;
		std::vector<std::unique_ptr<ASTBase>> Vector_ClassDataProtected;

		while (CurrToken == HazeToken::ClassPublic || CurrToken == HazeToken::ClassPrivate || CurrToken == HazeToken::ClassProtected)
		{
			if (CurrToken == HazeToken::ClassPublic)
			{
				if (Vector_ClassDataPublic.size() == 0)
				{
					GetNextToken();
					while (CurrToken != HazeToken::RightBrace)
					{
						GetNextToken();
						Vector_ClassDataPublic.push_back(ParseVariableDefine());
					}

					GetNextToken();
				}
			}
			else if (CurrToken == HazeToken::ClassPrivate)
			{
				if (Vector_ClassDataPrivate.size() == 0)
				{
					GetNextToken();
					while (CurrToken != HazeToken::RightBrace)
					{
						GetNextToken();
						Vector_ClassDataPrivate.push_back(ParseVariableDefine());
					}

					GetNextToken();
				}
			}
			else if (CurrToken == HazeToken::ClassProtected)
			{
				if (Vector_ClassDataProtected.size() == 0)
				{
					GetNextToken();
					while (CurrToken != HazeToken::RightBrace)
					{
						GetNextToken();
						Vector_ClassDataProtected.push_back(ParseVariableDefine());
					}

					GetNextToken();
				}
			}
		}

		Vector_ClassData.push_back(std::move(Vector_ClassDataPublic));
		Vector_ClassData.push_back(std::move(Vector_ClassDataPrivate));
		Vector_ClassData.push_back(std::move(Vector_ClassDataProtected));
	
		GetNextToken();
	}

	return Vector_ClassData;
}

std::unique_ptr<ASTClassFunctionSection> Parse::ParseClassFunction(const HAZE_STRING& ClassName)
{
	GetNextToken();

	std::vector<std::vector<std::unique_ptr<ASTFunction>>> Vector_ClassFunction;

	while (CurrToken == HazeToken::ClassPublic || CurrToken == HazeToken::ClassPrivate || CurrToken == HazeToken::ClassProtected)
	{
		std::vector<std::unique_ptr<ASTFunction>> Vector_FunctionPublic;
		std::vector<std::unique_ptr<ASTFunction>> Vector_FunctionPrivate;
		std::vector<std::unique_ptr<ASTFunction>> Vector_FunctionProtected;


		if (CurrToken == HazeToken::ClassPublic)
		{
			if (Vector_FunctionPublic.size() == 0)
			{
				GetNextToken();
				while (CurrToken != HazeToken::RightBrace)
				{
					GetNextToken();
					Vector_FunctionPublic.push_back(ParseFunction(&ClassName));
				}

				GetNextToken();
			}
		}
		else if (CurrToken == HazeToken::ClassPrivate)
		{
			if (Vector_FunctionPrivate.size() == 0)
			{
				GetNextToken();
				while (CurrToken != HazeToken::RightBrace)
				{
					GetNextToken();
					Vector_FunctionPrivate.push_back(ParseFunction(&ClassName));
				}

				GetNextToken();
			}
		}
		else if (CurrToken == HazeToken::ClassProtected)
		{
			if (Vector_FunctionProtected.size() == 0)
			{
				GetNextToken();
				while (CurrToken != HazeToken::RightBrace)
				{
					GetNextToken();
					Vector_FunctionProtected.push_back(ParseFunction(&ClassName));
				}

				GetNextToken();
			}
		}

	}

	GetNextToken();
	return std::make_unique<ASTClassFunctionSection>(VM, Vector_ClassFunction);
}

bool Parse::ExpectNextTokenIs(HazeToken Token, const wchar_t* ErrorInfo)
{
	HazeToken NextToken = GetNextToken();
	if (Token != NextToken)
	{
		if (ErrorInfo)
		{
			HazeLog::LogInfo(HazeLog::Error, ErrorInfo);
		}
		return false;
	}

	return true;
}

bool Parse::TokenIs(HazeToken Token, const wchar_t* ErrorInfo)
{
	if (Token != CurrToken)
	{
		if (ErrorInfo)
		{
			HazeLog::LogInfo(HazeLog::Error, ErrorInfo);
		}
		return false;
	}

	return true;
}

bool Parse::IsHazeSignalToken(HAZE_CHAR Char)
{
	static std::unordered_set<HAZE_STRING> SetTokenText =
	{
		TOKEN_ADD, TOKEN_SUB, TOKEN_MUL, TOKEN_DIV, TOKEN_MOD, TOKEN_LEFT_MOVE, TOKEN_RIGHT_MOVE,
		TOKEN_ASSIGN, TOKEN_EQUAL, TOKEN_NOT_EQUAL, TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL,
		TOKEN_LEFT_PARENTHESES, TOKEN_RIGHT_PARENTHESES, TOKEN_COMMA, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, HAZE_SINGLE_COMMENT, 
		TOKEN_MULTI_VARIABLE, TOKEN_STRING_START, TOKEN_STRING_END
	};

	HAZE_STRING WS;
	WS = Char;
	return SetTokenText.find(WS) != SetTokenText.end();
}

bool Parse::IsPointer(const HAZE_STRING& Str)
{
	HAZE_STRING TypeName = Str.substr(0, Str.length() - 1);
	HAZE_STRING PointerChar = Str.substr(Str.length() - 1, 1);

	if (PointerChar == TOKEN_MUL)
	{
		auto It = MapToken.find(TypeName);
		if (It != MapToken.end())
		{
			IsHazeDefaultType(GetValueTypeByToken(It->second));
		}
		else
		{
			return VM->GetCompiler()->IsClass(TypeName);
		}
		
	}
	return false;
}
