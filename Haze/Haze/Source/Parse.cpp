#include <cctype>
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

#define HAZE_SINGLE_COMMENT				HAZE_TEXT("//")
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

#define TOKEN_ARRAY						HAZE_TEXT("数组")
#define TOKEN_ARRAY_START				HAZE_TEXT("[")
#define TOKEN_ARRAY_END					HAZE_TEXT("]")

#define TOKEN_CHAR						HAZE_TEXT("字符")
#define TOKEN_STRING_MATCH				HAZE_TEXT("\"")

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

#define TOKEN_AND						HAZE_TEXT("且")		//&&
#define TOKEN_OR						HAZE_TEXT("或")		//||
#define TOKEN_NOT						HAZE_TEXT("非")		//!

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

#define TOKEN_ADD_ASSIGN				HAZE_TEXT("+=")
#define TOKEN_SUB_ASSIGN				HAZE_TEXT("-=")
#define TOKEN_MUL_ASSIGN				HAZE_TEXT("*=")
#define TOKEN_DIV_ASSIGN				HAZE_TEXT("/=")
#define TOKEN_MOD_ASSIGN				HAZE_TEXT("%=")
#define TOKEN_SHL_ASSIGN				HAZE_TEXT("<<=")
#define TOKEN_SHR_ASSIGN				HAZE_TEXT(">>=")

#define TOKEN_LEFT_PARENTHESES			HAZE_TEXT("(")
#define TOKEN_RIGHT_PARENTHESES			HAZE_TEXT(")")

#define TOKEN_COMMA						HAZE_TEXT(",")

#define TOKEN_LEFT_BRACE				HAZE_TEXT("{")
#define TOKEN_RIGHT_BRACE				HAZE_TEXT("}")

#define TOKEN_IF						HAZE_TEXT("若")
#define TOKEN_ELSE						HAZE_TEXT("否则")

#define TOKEN_FOR						HAZE_TEXT("循环")
#define TOKEN_FOR_STEP					HAZE_TEXT("步进")

#define TOKEN_BREAK						HAZE_TEXT("打断")
#define TOKEN_CONTINUE					HAZE_TEXT("继续")
#define TOKEN_RETURN					HAZE_TEXT("返")

#define TOKEN_WHILE						HAZE_TEXT("当")

#define TOKEN_CAST						HAZE_TEXT("转")

#define TOKEN_REFERENCE					HAZE_TEXT("引用")

#define TOKEN_DEFINE					HAZE_TEXT("定义")

#define TOKEN_STANDARD_LIBRARY			HAZE_TEXT("标准库")
#define TOKEN_IMPORT_MODULE				HAZE_TEXT("引")

#define TOKEN_MULTI_VARIABLE			HAZE_TEXT("...")

#define TOKEN_NEW						HAZE_TEXT("生成")

static std::unordered_map<HAZE_STRING, HazeToken> HashMap_Token =
{
	{TOKEN_VOID, HazeToken::Void},
	{TOKEN_BOOL, HazeToken::Bool},
	{TOKEN_CHAR, HazeToken::Char},
	{TOKEN_INT, HazeToken::Int},
	{TOKEN_FLOAT, HazeToken::Float},
	{TOKEN_LONG, HazeToken::Long},
	{TOKEN_DOUBLE, HazeToken::Double},

	{TOKEN_UNSIGNED_INT, HazeToken::UnsignedInt},
	{TOKEN_UNSIGNED_LONG, HazeToken::UnsignedLong},

	{TOKEN_ARRAY, HazeToken::Array},

	{TOKEN_STRING_MATCH, HazeToken::StringMatch},

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

	{TOKEN_LEFT_MOVE, HazeToken::Shl},
	{TOKEN_RIGHT_MOVE, HazeToken::Shr},

	{TOKEN_ASSIGN, HazeToken::Assign},
	{TOKEN_EQUAL, HazeToken::Equal},
	{TOKEN_NOT_EQUAL, HazeToken::NotEqual},
	{TOKEN_GREATER, HazeToken::Greater},
	{TOKEN_GREATER_EQUAL, HazeToken::GreaterEqual},
	{TOKEN_LESS, HazeToken::Less},
	{TOKEN_LESS_EQUAL, HazeToken::LessEqual},

	{TOKEN_INC, HazeToken::Inc},
	{TOKEN_DEC, HazeToken::Dec},
	{TOKEN_ADD_ASSIGN, HazeToken::AddAssign},
	{TOKEN_SUB_ASSIGN, HazeToken::SubAssign},
	{TOKEN_MUL_ASSIGN, HazeToken::MulAssign},
	{TOKEN_DIV_ASSIGN, HazeToken::DivAssign},
	{TOKEN_MOD_ASSIGN, HazeToken::ModAssign},
	{TOKEN_SHL_ASSIGN, HazeToken::ShlAssign},
	{TOKEN_SHR_ASSIGN, HazeToken::ShrAssign},

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
	{ HazeToken::Assign, 100 },
	{ HazeToken::AddAssign, 100 },
	{ HazeToken::SubAssign, 100 },
	{ HazeToken::MulAssign, 100 },
	{ HazeToken::DivAssign, 100 },
	{ HazeToken::ModAssign, 100 },
	{ HazeToken::BitAndAssign, 100 },
	{ HazeToken::BitOrAssign, 100 },
	{ HazeToken::BitXorAssign, 100 },
	{ HazeToken::ShlAssign, 100 },
	{ HazeToken::ShrAssign, 100 },


	{ HazeToken::Or, 140 },
	{ HazeToken::And, 150 },

	{ HazeToken::Equal, 200 },
	{ HazeToken::NotEqual, 200 },

	{ HazeToken::Greater, 250 },
	{ HazeToken::GreaterEqual, 250 },
	{ HazeToken::Less, 250 },
	{ HazeToken::LessEqual, 250 },

	{ HazeToken::Shl, 300 },
	{ HazeToken::Shr, 300 },


	{ HazeToken::Add, 400 },
	{ HazeToken::Sub, 400 },

	{ HazeToken::Mul, 500 },
	{ HazeToken::Div, 500 },
	{ HazeToken::Mod, 500 },

	//{ HazeToken::Not, 60 },
	//{ HazeToken::Inc, 60 },
	// 
	//{ HazeToken::LeftParentheses, 1000 },
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
		case HazeToken::Char:
		case HazeToken::Int:
		case HazeToken::Float:
		case HazeToken::Long:
		case HazeToken::Double:
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
	/*if (!*CurrCode)
	{
		CurrToken = HazeToken::None;
		CurrLexeme = HAZE_TEXT("");
		return HazeToken::None;
	}*/

	while (HazeIsSpace(*CurrCode))
	{
		CurrCode++;
	}

	if (HAZE_STRING(CurrCode) == HAZE_TEXT(""))
	{
		CurrToken = HazeToken::None;
		CurrLexeme = HAZE_TEXT("");
		return HazeToken::None;
	}

	//Match Token
	CurrLexeme.clear();
	const HAZE_CHAR* Signal;
	while (!HazeIsSpace(*CurrCode) || CurrToken == HazeToken::StringMatch)
	{
		if (IsHazeSignalToken(CurrCode, Signal))
		{
			if (CurrToken == HazeToken::StringMatch)
			{
				static HAZE_STRING TempString;
				TempString = *(CurrCode++);
				auto Iter = HashMap_Token.find(TempString);
				if (Iter != HashMap_Token.end() && Iter->second == HazeToken::StringMatch)
				{
					CurrToken = HazeToken::None;
					return CurrToken;
				}
				else
				{
					CurrLexeme += TempString;
				}
				continue;
			}
			else if (HAZE_STRING(Signal) == TOKEN_ARRAY_START)
			{
				if (CurrLexeme.empty())
				{
					CurrLexeme += *CurrCode++;
					CurrToken = HazeToken::Array;
					return CurrToken;
				}
			}
			else if (IsHazeSignalToken(CurrCode, Signal, 3))
			{
				CurrPreLexeme = CurrLexeme;
				CurrLexeme = Signal;
				CurrCode += 3;
			}
			else if (IsHazeSignalToken(CurrCode, Signal, 2))
			{
				CurrPreLexeme = CurrLexeme;
				CurrLexeme = Signal;
				CurrCode += 2;
			}
			else if (CurrLexeme.length() == 0 || IsPointer(CurrLexeme + *CurrCode, CurrToken))
			{
				CurrLexeme += *(CurrCode++);
			}
			break;
		}

		CurrLexeme += *(CurrCode++);
	}

	static HAZE_STRING Comment_Str;
	Comment_Str = *CurrCode;
	if (CurrLexeme == HAZE_SINGLE_COMMENT)
	{
		while (Comment_Str != HAZE_TEXT("\n"))
		{
			CurrCode++;
			Comment_Str = *CurrCode;
		}
		return GetNextToken();
	}
	else if (CurrLexeme == HAZE_MULTI_COMMENT_START)
	{
		Comment_Str.resize(2);
		while (Comment_Str != HAZE_MULTI_COMMENT_END)
		{
			CurrCode++;
			memcpy(Comment_Str.data(), CurrCode, sizeof(HAZE_CHAR) * 2);
		}

		CurrCode += 2;
		return GetNextToken();
	}

	auto It = HashMap_Token.find(CurrLexeme);
	if (It != HashMap_Token.end())
	{
		CurrToken = It->second;
	}
	else if (IsNumber(CurrLexeme))
	{
		CurrToken = HazeToken::Number;
	}
	else if (IsPointer(CurrLexeme, CurrToken))
	{
	}
	else if (VM->IsClass(CurrLexeme) || CurrParseClass == CurrLexeme)
	{
		CurrToken = HazeToken::CustomClass;
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

std::unique_ptr<ASTBase> Parse::ParseUnaryExpression(bool HasLeft)
{
	auto it = MapBinOp.find(CurrToken);
	if (it == MapBinOp.end() || !HasLeft)
	{
		return ParsePrimary();
	}
	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseBinaryOperateExpression(int Prec, std::unique_ptr<ASTBase> Left)
{
	while (true)
	{
		auto it = MapBinOp.find(CurrToken);
		if (it == MapBinOp.end())
		{
			return Left;
		}

		if (it->second < Prec)
		{
			return Left;
		}

		HazeToken OpToken = CurrToken;

		GetNextToken();

		std::unique_ptr<ASTBase> Right = ParseUnaryExpression(true);
		if (!Right)
		{
			return nullptr;
		}

		auto NextPrec = MapBinOp.find(CurrToken);
		if (NextPrec == MapBinOp.end())
		{
			return std::make_unique<ASTBinaryExpression>(VM, StackSectionSignal.top(), OpToken, Left, Right);
		}

		if (it->second < NextPrec->second)
		{
			Right = ParseBinaryOperateExpression(it->second + 1, std::move(Right));
			if (!Right)
			{
				return nullptr;
			}

			/*NextPrec = MapBinOp.find(CurrToken);
			if (NextPrec == MapBinOp.end())
			{
				return std::make_unique<ASTBinaryExpression>(VM, StackSectionSignal.top(), OpToken, Left, Right);
			}*/
		}

		// Merge LHS/RHS.
		Left = std::make_unique<ASTBinaryExpression>(VM, StackSectionSignal.top(), OpToken, Left, Right);
	}
}

std::unique_ptr<ASTBase> Parse::ParsePrimary()
{
	HazeToken Token = CurrToken;
	switch (Token)
	{
	case HazeToken::Bool:
	case HazeToken::Char:
	case HazeToken::Int:
	case HazeToken::Float:
	case HazeToken::Long:
	case HazeToken::Double:
	case HazeToken::UnsignedInt:
	case HazeToken::UnsignedLong:
	case HazeToken::String:
	case HazeToken::CustomClass:
	case HazeToken::PointerBase:
	case HazeToken::PointerClass:
		return ParseVariableDefine();
	case HazeToken::Number:
		return ParseNumberExpression();
	case HazeToken::StringMatch:
		return ParseStringText();
	case HazeToken::True:
	case HazeToken::False:
		return ParseBoolExpression();
	case HazeToken::Identifier:
		return ParseIdentifer();
	case HazeToken::If:
		return ParseIfExpression();
	case HazeToken::While:
		return ParseWhileExpression();
	case HazeToken::For:
		return ParseForExpression();
	case HazeToken::Return:
		return ParseReturn();
	case HazeToken::Inc:
		return ParseInc();
	case HazeToken::Dec:
		return ParseDec();
	case HazeToken::AddAssign:
	case HazeToken::SubAssign:
	case HazeToken::MulAssign:
	case HazeToken::DivAssign:
	case HazeToken::ModAssign:
	case HazeToken::BitAndAssign:
	case HazeToken::BitOrAssign:
	case HazeToken::BitXorAssign:
	case HazeToken::ShlAssign:
	case HazeToken::ShrAssign:
		return ParseOperatorAssign();
	case HazeToken::New:
		return ParseNew();
	case HazeToken::LeftParentheses:
		return ParseLeftParentheses();
	case HazeToken::LeftBrace:
		return ParseLeftBrace();
	case HazeToken::Mul:					//pointer value
		return ParsePointerValue();
	default:
		break;
	}

	return nullptr;
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
	else if (CurrToken == HazeToken::Array)
	{
		GetNextToken();
		auto IndexExpression = ParseExpression();
		GetNextToken();

		return std::make_unique<ASTIdentifier>(VM, StackSectionSignal.top(), IdentiferName, nullptr, std::move(IndexExpression));
	}
	else
	{
		return std::make_unique<ASTIdentifier>(VM, StackSectionSignal.top(), IdentiferName);
	}
}

std::unique_ptr<ASTBase> Parse::ParseVariableDefine()
{
	DefineVariable.Type.PrimaryType = GetValueTypeByToken(CurrToken);
	if (CurrToken == HazeToken::CustomClass)
	{
		DefineVariable.Type.CustomName = CurrLexeme;
	}
	else if (CurrToken == HazeToken::PointerBase)
	{
		HazeToken Token = HashMap_Token.find(CurrLexeme.substr(0, CurrLexeme.length() - 1))->second;
		DefineVariable.Type.SecondaryType = GetValueTypeByToken(Token);
		DefineVariable.Type.CustomName = HAZE_TEXT("");
	}
	else if (CurrToken == HazeToken::PointerClass)
	{
		DefineVariable.Type.SecondaryType = HazeValueType::Class;
		DefineVariable.Type.CustomName = CurrLexeme.substr(0, CurrLexeme.length() - 1);
	}

	std::unique_ptr<ASTBase> ArraySize = nullptr;
	if (ExpectNextTokenIs(HazeToken::Identifier))
	{
		DefineVariable.Name = CurrLexeme;

		GetNextToken();

		DefineVariable.Type.IsArray = CurrToken == HazeToken::Array;
		if (DefineVariable.Type.IsArray)
		{
			DefineVariable.Type.SecondaryType = DefineVariable.Type.PrimaryType;
			DefineVariable.Type.PrimaryType = HazeValueType::Array;

			GetNextToken();
			ArraySize= std::move(ParseExpression());
			GetNextToken();
		}

		if (CurrToken == HazeToken::Assign)
		{
			GetNextToken();		//吃掉赋值符号
			std::unique_ptr<ASTBase> Expression = ParseExpression();

			return std::make_unique<ASTVariableDefine>(VM, StackSectionSignal.top(), DefineVariable, std::move(Expression), std::move(ArraySize));
		}
		else if ((CurrToken == HazeToken::RightParentheses || CurrToken == HazeToken::Comma) && StackSectionSignal.top() == HazeSectionSignal::Function)
		{
			//函数调用
			return std::make_unique<ASTVariableDefine>(VM, StackSectionSignal.top(), DefineVariable, nullptr);
		}
		else if (DefineVariable.Type.PrimaryType == HazeValueType::Class )
		{
			if (CurrToken == HazeToken::LeftParentheses)
			{
				//类对象定义
				GetNextToken();
				if (CurrToken == HazeToken::RightParentheses)
				{
					GetNextToken();
					return std::make_unique<ASTVariableDefine>(VM, StackSectionSignal.top(), DefineVariable, nullptr);
				}
				else
				{
					//自定义构造函数
					/*while (CurrToken != HazeToken::RightParentheses)
					{

					}*/
				}
			}
			else
			{
				HAZE_LOG_ERR(HAZE_TEXT("Class object define need left parentheses!\n"));
			}
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

	GetNextToken();
	return std::make_unique<ASTStringText>(VM, Text);
}

std::unique_ptr<ASTBase> Parse::ParseBoolExpression()
{
	HazeValue Value;
	Value.Value.Bool = CurrLexeme == TOKEN_TRUE;

	return std::make_unique<ASTBool>(VM, Value);
}

std::unique_ptr<ASTBase> Parse::ParseNumberExpression()
{
	HazeValue Value;
	HazeValueType Type = GetNumberDefaultType(CurrLexeme);

	StringToHazeValueNumber(CurrLexeme, Type, Value);

	GetNextToken();
	return std::make_unique<ASTNumber>(VM, Type, Value);
}

std::unique_ptr<ASTBase> Parse::ParseIfExpression(bool Recursion)
{
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("解析错误：若 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto ConditionExpression = ParseExpression();

		std::unique_ptr<ASTBase> IfMultiExpression = nullptr;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("解析错误：若 执行表达式期望捕捉 {")))
		{
			IfMultiExpression = ParseMultiExpression();
			GetNextToken();
		}
		
		std::unique_ptr<ASTBase> ElseExpression = nullptr;

		
		if (CurrToken == HazeToken::Else)
		{
			GetNextToken();

			bool NextNotIf = CurrToken != HazeToken::If;
			if (NextNotIf)
			{
				ElseExpression = ParseMultiExpression();
			}
			else
			{
				ElseExpression = ParseIfExpression(true);
			}

			if (!Recursion)
			{
				GetNextToken();
			}
		}

		return std::make_unique<ASTIfExpression>(VM, ConditionExpression, IfMultiExpression, ElseExpression);
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseForExpression()
{
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("解析错误：循环 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto InitExpression = ParseExpression();

		GetNextToken();
		auto ConditionExpression = ParseExpression();

		GetNextToken();
		auto StepExpression = ParseExpression();

		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("解析错误：循环 表达式块期望捕捉 {")))
		{
			auto MultiExpression = ParseMultiExpression();

			GetNextToken();
			return std::make_unique<ASTForExpression>(VM, InitExpression, ConditionExpression, StepExpression, MultiExpression);
		}
	}
	
	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseWhileExpression()
{
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("解析错误：当 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto ConditionExpression = ParseExpression();

		std::unique_ptr<ASTBase> MultiExpression = nullptr;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("解析错误：当 执行表达式期望捕捉 {")))
		{
			MultiExpression = ParseMultiExpression();
			
			GetNextToken();
			return std::make_unique<ASTWhileExpression>(VM, ConditionExpression, MultiExpression);
		}
	}

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
	
	Define.Type.PrimaryType = GetValueTypeByToken(CurrToken);
	if (Define.Type.PrimaryType == HazeValueType::Class)
	{
		Define.Type.PrimaryType = HazeValueType::PointerClass;
		Define.Type.CustomName = CurrLexeme;
	}
	else
	{
		Define.Type.SecondaryType = Define.Type.PrimaryType;
		Define.Type.PrimaryType = HazeValueType::PointerBase;
	}

	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("New expression expect (\n")))
	{
		if (ExpectNextTokenIs(HazeToken::RightParentheses, HAZE_TEXT("New expression expect )\n"))) 
		{
			GetNextToken();
			return std::make_unique<ASTNew>(VM, Define);
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseInc()
{
	HAZE_STRING Name;
	bool IsPreInc = !CurrPreLexeme.empty();
	
	if (IsPreInc)
	{
		Name = CurrPreLexeme;
		CurrPreLexeme.clear();
	}
	else
	{
		GetNextToken();
		Name = CurrLexeme;
	}

	GetNextToken();
	return std::make_unique<ASTInc>(VM, Name, IsPreInc);
}

std::unique_ptr<ASTBase> Parse::ParseDec()
{
	HAZE_STRING Name;
	bool IsPreInc = !CurrPreLexeme.empty();

	if (IsPreInc)
	{
		Name = CurrPreLexeme;
		CurrPreLexeme.clear();
	}
	else
	{
		GetNextToken();
		Name = CurrLexeme;
	}

	GetNextToken();
	return std::make_unique<ASTDec>(VM, Name, IsPreInc);
}

std::unique_ptr<ASTBase> Parse::ParseLeftParentheses()
{
	GetNextToken();
	auto Expression = ParseExpression();
	if (CurrToken == HazeToken::RightParentheses)
	{
		GetNextToken();
		return Expression;
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("Parse left parentheses error, expect right parentheses\n"));
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParsePointerValue()
{
	GetNextToken();
	auto Expression = ParseExpression();
	return std::make_unique<ASTPointerValue>(VM, Expression);
}

std::unique_ptr<ASTBase> Parse::ParseLeftBrace()
{
	std::vector<std::unique_ptr<ASTBase>> Vector_Element;

	GetNextToken();
	while (true)
	{
		Vector_Element.push_back(ParseExpression());

		if (CurrToken == HazeToken::RightBrace)
		{
			GetNextToken();
			break;
		}
		else if(CurrToken == HazeToken::Comma)
		{
			GetNextToken();
		}
	}

	return std::make_unique<ASTInitializeList>(VM, Vector_Element);
}

std::unique_ptr<ASTBase> Parse::ParseOperatorAssign()
{
	HazeToken Token = CurrToken;

	auto Value = ParseExpression();

	GetNextToken();
	return std::make_unique<ASTOperetorAssign>(VM, Token, Value);
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
			GetNextToken();
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
	HazeDefineType FuncType;
	FuncType.PrimaryType = GetValueTypeByToken(CurrToken);
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
				ThisParam.Type.PrimaryType = HazeValueType::PointerClass;
				ThisParam.Type.CustomName = ClassName->c_str();
				
				Vector_Param.push_back(ThisParam);
			}

			while (!TokenIs(HazeToken::LeftBrace))
			{
				HazeDefineVariable Param;
				if (GetNextToken() == HazeToken::RightParentheses)
				{
					break;
				}

				Param.Type.PrimaryType = GetValueTypeByToken(CurrToken);
				if (CurrToken == HazeToken::CustomClass)
				{
					Param.Type.PrimaryType = HazeValueType::Class;//此处有待优化
					Param.Type.CustomName = CurrLexeme;
				}

				if (Param.Type.PrimaryType == HazeValueType::MultiVariable)
				{
					ExpectNextTokenIs(HazeToken::RightParentheses, HAZE_TEXT("Error: Parse function expression expect function multiply param right need ) \n"));
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
	else if (*ClassName == FuncType.CustomName)
	{
		//类构造函数
		HAZE_STRING FunctionName = FuncType.CustomName;
		if (CurrToken == HazeToken::LeftParentheses)
		{
			std::vector<HazeDefineVariable> Vector_Param;

			if (ClassName && !ClassName->empty())
			{
				HazeDefineVariable ThisParam;
				ThisParam.Name = HAZE_CLASS_THIS;
				ThisParam.Type.PrimaryType = HazeValueType::PointerClass;
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

				Param.Type.PrimaryType = GetValueTypeByToken(CurrToken);
				if (CurrToken == HazeToken::Identifier)
				{
					Param.Type.PrimaryType = HazeValueType::Class;
					Param.Type.CustomName = CurrLexeme;
				}

				if (Param.Type.PrimaryType == HazeValueType::MultiVariable)
				{
					ExpectNextTokenIs(HazeToken::RightParentheses, HAZE_TEXT("Error: Parse class construction function expression expect function multiply param right need ) \n"));
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

			if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("Error: Parse class construction function body expect { \n")))
			{
				std::unique_ptr<ASTBase> Body = ParseMultiExpression();

				if (TokenIs(HazeToken::RightBrace, HAZE_TEXT("Error: Parse class construction function expression expect function body expect } \n")))
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

			Param.Type.PrimaryType = GetValueTypeByToken(CurrToken);
			if (CurrToken == HazeToken::Identifier)
			{
				Param.Type.PrimaryType = HazeValueType::Class;
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

			if (TokenIs(HazeToken::RightBrace, HAZE_TEXT("Error: Parse main function expression expect function body expect } \n")))
			{
				StackSectionSignal.pop();

				GetNextToken();
				HazeDefineType DefineType = { HazeValueType::Int, HAZE_TEXT("") };
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
	CurrParseClass = CurrLexeme;
	CurrParseClass.clear();
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
			HazeDefineType FuncType;
			FuncType.PrimaryType = GetValueTypeByToken(CurrToken);
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

						Param.Type.PrimaryType = GetValueTypeByToken(CurrToken);
						if (CurrToken == HazeToken::Identifier)
						{
							Param.Type.PrimaryType = HazeValueType::Class;
							Param.Type.CustomName = CurrLexeme;
						}
						else if (CurrToken == HazeToken::PointerBase)
						{
							Param.Type.SecondaryType = GetValueTypeByToken(HashMap_Token.find(CurrLexeme.substr(0, CurrLexeme.length() - 1))->second);
						}
						else if (CurrToken == HazeToken::PointerClass)
						{
							Param.Type.CustomName = CurrLexeme.substr(0, CurrLexeme.length() - 1);
						}

						if (Param.Type.PrimaryType == HazeValueType::MultiVariable)
						{
							ExpectNextTokenIs(HazeToken::RightParentheses, HAZE_TEXT("Error: Parse function expression expect function multiply param right need ) \n"));
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

		GetNextToken();

		//return std::make_unique<ASTStandardLibrary>(VM, StandardLibraryName, Vector_FunctionDefine);
	}

	return Vector_FunctionDefine;
}

std::unique_ptr<ASTBase> Parse::ParseImportModule()
{
	if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("Parse import module name is error\n")))
	{
		HAZE_STRING Name = CurrLexeme;
		GetNextToken();
		return std::make_unique<ASTImportModule>(VM, Name);
	}
	
	return nullptr;
}

std::unique_ptr<ASTClass> Parse::ParseClass()
{
	if (ExpectNextTokenIs(HazeToken::Identifier, (HAZE_TEXT("expect correct class name not ") + CurrLexeme).c_str()))
	{
		CurrParseClass = CurrLexeme;
		HAZE_STRING Name = CurrLexeme;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("expect { ")))
		{
			StackSectionSignal.push(HazeSectionSignal::Class);

			std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> Vector_ClassData;
			std::unique_ptr<ASTClassFunctionSection> ClassFunction;

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
						HAZE_LOG_ERR(HAZE_TEXT("class only one data define section  class :%ws \n"), Name.c_str());
					}
				}
				else if (CurrToken == HazeToken::Function)
				{
					ClassFunction = ParseClassFunction(Name);
				}
			}

			StackSectionSignal.pop();

			GetNextToken();

			CurrParseClass.clear();
			return std::make_unique<ASTClass>(VM, Name, Vector_ClassData, ClassFunction);
		}
	}

	return nullptr;
}

std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> Parse::ParseClassData()
{
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> Vector_ClassData;

	auto HasData = [](decltype(Vector_ClassData)& Vector_Data, HazeDataDesc ScopeType) -> bool
	{
		for (size_t i = 0; i < Vector_Data.size(); i++)
		{
			if (Vector_Data[i].first == ScopeType)
			{
				return true;
			}
		}
		return false;
	};

	if (ExpectNextTokenIs(HazeToken::LeftBrace, (HAZE_TEXT("expect { ") + CurrLexeme).c_str()))
	{
		GetNextToken();

		while (CurrToken == HazeToken::ClassPublic || CurrToken == HazeToken::ClassPrivate || CurrToken == HazeToken::ClassProtected)
		{
			if (CurrToken == HazeToken::ClassPublic)
			{
				if (!HasData(Vector_ClassData, HazeDataDesc::ClassMember_Local_Public))
				{
					Vector_ClassData.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>( { HazeDataDesc::ClassMember_Local_Public,
						std::move(std::vector<std::unique_ptr<ASTBase>>{} )})));

					GetNextToken();
					GetNextToken();
					while (CurrToken != HazeToken::RightBrace)
					{
						Vector_ClassData.back().second.push_back(ParseVariableDefine());
					}

					GetNextToken();
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("Class scope only once for public"));
				}
			}
			else if (CurrToken == HazeToken::ClassPrivate)
			{
				if (!HasData(Vector_ClassData, HazeDataDesc::ClassMember_Local_Private))
				{
					Vector_ClassData.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Private,
						std::move(std::vector<std::unique_ptr<ASTBase>>{}) })));

					GetNextToken();
					while (CurrToken != HazeToken::RightBrace)
					{
						GetNextToken();
						Vector_ClassData.back().second.push_back(ParseVariableDefine());
					}

					GetNextToken();
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("Class scope only once for private"));
				}
			}
			else if (CurrToken == HazeToken::ClassProtected)
			{
				if (!HasData(Vector_ClassData, HazeDataDesc::ClassMember_Local_Protected))
				{
					Vector_ClassData.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Protected,
						std::move(std::vector<std::unique_ptr<ASTBase>>{}) })));

					GetNextToken();
					while (CurrToken != HazeToken::RightBrace)
					{
						GetNextToken();
						Vector_ClassData.back().second.push_back(ParseVariableDefine());
					}

					GetNextToken();
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("Class scope only once for protected"));
				}
			}
		}

		if (CurrToken != HazeToken::RightBrace)
		{
			HAZE_LOG_ERR(HAZE_TEXT("Parse class data section end error \n"));
		}

		GetNextToken();
	}

	return Vector_ClassData;
}

std::unique_ptr<ASTClassFunctionSection> Parse::ParseClassFunction(const HAZE_STRING& ClassName)
{
	if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("class function expect { ")))
	{
		GetNextToken();

		std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>> Vector_ClassFunction;
		//	={ {HazeDataDesc::ClassFunction_Local_Public, {}} };

		auto HasData = [](decltype(Vector_ClassFunction)& Vector_Function, HazeDataDesc ScopeType) -> bool
		{
			for (size_t i = 0; i < Vector_Function.size(); i++)
			{
				if (Vector_Function[i].first == ScopeType)
				{
					return true;
				}
			}
			return false;
		};

		while (CurrToken == HazeToken::ClassPublic || CurrToken == HazeToken::ClassPrivate || CurrToken == HazeToken::ClassProtected)
		{
			if (CurrToken == HazeToken::ClassPublic)
			{
				if (!(HasData(Vector_ClassFunction, HazeDataDesc::ClassFunction_Local_Public)))
				{
					Vector_ClassFunction.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Public,
						std::move(std::vector<std::unique_ptr<ASTFunction>>{}) })));

					GetNextToken();
					GetNextToken();
					while (CurrToken != HazeToken::RightBrace)
					{
						Vector_ClassFunction.back().second.push_back(ParseFunction(&ClassName));
					}

					GetNextToken();
				}
			}
			else if (CurrToken == HazeToken::ClassPrivate)
			{
				if (!(HasData(Vector_ClassFunction, HazeDataDesc::ClassFunction_Local_Private)))
				{
					Vector_ClassFunction.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Private,
						std::move(std::vector<std::unique_ptr<ASTFunction>>{}) })));

					GetNextToken();
					GetNextToken();

					while (CurrToken != HazeToken::RightBrace)
					{
						Vector_ClassFunction.back().second.push_back(ParseFunction(&ClassName));
					}

					GetNextToken();
				}
			}
			else if (CurrToken == HazeToken::ClassProtected)
			{
				if (!(HasData(Vector_ClassFunction, HazeDataDesc::ClassFunction_Local_Protected)))
				{
					Vector_ClassFunction.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Protected,
						std::move(std::vector<std::unique_ptr<ASTFunction>>{}) })));

					GetNextToken();
					GetNextToken();

					while (CurrToken != HazeToken::RightBrace)
					{
						Vector_ClassFunction.back().second.push_back(ParseFunction(&ClassName));
					}

					GetNextToken();
				}
			}

		}

		if (CurrToken != HazeToken::RightBrace)
		{
			HAZE_LOG_ERR(HAZE_TEXT("Parse class function section end error \n"));
		}

		GetNextToken();
		return std::make_unique<ASTClassFunctionSection>(VM, Vector_ClassFunction);
	}

	return nullptr;
}

bool Parse::ExpectNextTokenIs(HazeToken Token, const HAZE_CHAR* ErrorInfo)
{
	HazeToken NextToken = GetNextToken();
	if (Token != NextToken)
	{
		if (ErrorInfo)
		{
			HAZE_LOG_ERR(ErrorInfo);
		}
		return false;
	}

	return true;
}

bool Parse::TokenIs(HazeToken Token, const HAZE_CHAR* ErrorInfo)
{
	if (Token != CurrToken)
	{
		if (ErrorInfo)
		{
			HAZE_LOG_ERR(ErrorInfo);
		}
		return false;
	}

	return true;
}

bool Parse::IsHazeSignalToken(const HAZE_CHAR* Char, const HAZE_CHAR*& OutChar, uint32 CharSize)
{
	static std::unordered_set<HAZE_STRING> HashSet_TokenText =
	{
		TOKEN_ADD, TOKEN_SUB, TOKEN_MUL, TOKEN_DIV, TOKEN_MOD, 
		TOKEN_LEFT_MOVE, TOKEN_RIGHT_MOVE,
		TOKEN_ASSIGN, 
		TOKEN_EQUAL, TOKEN_NOT_EQUAL, TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL,
		TOKEN_LEFT_PARENTHESES, TOKEN_RIGHT_PARENTHESES, 
		TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, 
		HAZE_SINGLE_COMMENT, HAZE_MULTI_COMMENT_START, HAZE_MULTI_COMMENT_END,
		TOKEN_COMMA, 
		TOKEN_MULTI_VARIABLE, 
		TOKEN_STRING_MATCH,
		TOKEN_ARRAY_START, TOKEN_ARRAY_END,
		TOKEN_INC, TOKEN_DEC, TOKEN_ADD_ASSIGN, TOKEN_SUB_ASSIGN, TOKEN_MUL_ASSIGN, TOKEN_DIV_ASSIGN, TOKEN_MOD_ASSIGN, TOKEN_SHL_ASSIGN, TOKEN_SHR_ASSIGN
	};

	static HAZE_STRING WS;
	
	WS.resize(CharSize);
	memcpy(WS.data(), Char, sizeof(HAZE_CHAR) * CharSize);

	auto Iter = HashSet_TokenText.find(WS);
	if (Iter != HashSet_TokenText.end())
	{
		OutChar = Iter->c_str();
	}
	
	return Iter != HashSet_TokenText.end();
}

bool Parse::IsPointer(const HAZE_STRING& Str, HazeToken& OutToken)
{
	HAZE_STRING TypeName = Str.substr(0, Str.length() - 1);
	HAZE_STRING PointerChar = Str.substr(Str.length() - 1, 1);

	if (PointerChar == TOKEN_MUL)
	{
		auto It = HashMap_Token.find(TypeName);
		if (It != HashMap_Token.end())
		{
			if (IsHazeDefaultType(GetValueTypeByToken(It->second)))
			{
				OutToken = HazeToken::PointerBase;
				return true;
			}
		}
		else
		{
			if (VM->GetCompiler()->IsClass(TypeName))
			{
				OutToken = HazeToken::PointerClass;
				return true;
			}
		}
		
	}
	return false;
}
