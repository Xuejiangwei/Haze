#include <cctype>
#include <fstream>

#include "Haze.h"
#include "HazeVM.h"
#include "Parse.h"
#include "ASTBase.h"
#include "HazeLog.h"

#define HAZE_SINGLE_COMMENT				L"//"
#define HAZE_MUTIL_COMMENT_START		L"/*"
#define HAZE_MUTIL_COMMENT_END			L"*/"

#define TOKEN_BOOL				HAZE_TEXT("布尔")
#define TOKEN_CHAR				HAZE_TEXT("字符")
#define TOKEN_BYTE				HAZE_TEXT("字节")
#define TOKEN_SHORT				HAZE_TEXT("双字节")
#define TOKEN_INT				HAZE_TEXT("整数")
#define TOKEN_FLOAT				HAZE_TEXT("小数")
#define TOKEN_LONG				HAZE_TEXT("长整数")
#define TOKEN_DOUBLE			HAZE_TEXT("长小数")

#define TOKEN_UNSIGN_BYTE		HAZE_TEXT("正字节")
#define TOKEN_UNSIGN_SHORT		HAZE_TEXT("正双字节")
#define TOKEN_UNSIGN_INT		HAZE_TEXT("正整数")
#define TOKEN_UNSIGN_LONG		HAZE_TEXT("正长整数")

#define TOKEN_CLASS				HAZE_TEXT("类")
#define TOKEN_CLASS_DATA		HAZE_TEXT("类数据")
#define TOKEN_CLASS_FUNCTION	HAZE_TEXT("类函数")

#define TOKEN_TRUE				HAZE_TEXT("真")
#define TOKEN_FALSE				HAZE_TEXT("假")

#define TOKEN_ADD				HAZE_TEXT("+")
#define TOKEN_SUB				HAZE_TEXT("-")
#define TOKEN_MUL				HAZE_TEXT("*")
#define TOKEN_DIV				HAZE_TEXT("/")
#define TOKEN_MOD				HAZE_TEXT("%")

#define TOKEN_AND				HAZE_TEXT("与")
#define TOKEN_OR				HAZE_TEXT("或")
#define TOKEN_NOT				HAZE_TEXT("非")

#define TOKEN_LEFT_MOVE			HAZE_TEXT("<<")
#define TOKEN_RIGHT_MOVE		HAZE_TEXT(">>")

#define TOKEN_ASSIGN			HAZE_TEXT("=")
#define TOKEN_EQUAL				HAZE_TEXT("==")
#define TOKEN_NOT_EQUAL			HAZE_TEXT("!=")
#define TOKEN_GREATER			HAZE_TEXT(">")
#define TOKEN_GREATER_EQUAL		HAZE_TEXT(">=")
#define TOKEN_LESS				HAZE_TEXT("<")
#define TOKEN_LESS_EQUAL		HAZE_TEXT("<=")

#define TOKEN_LEFT_PARENTHESES	HAZE_TEXT("(")
#define TOKEN_RIGHT_PARENTHESES	HAZE_TEXT(")")

#define TOKEN_LEFT_BRACE		HAZE_TEXT("{")
#define TOKEN_RIGHT_BRACE		HAZE_TEXT("}")

//
#define TOKEN_IF				HAZE_TEXT("若")
#define TOKEN_ELSE				HAZE_TEXT("否则")

#define TOKEN_FOR				HAZE_TEXT("遍历")
#define TOKEN_FOR_STEP			HAZE_TEXT("步进")

#define TOKEN_BREAK				HAZE_TEXT("打断")
#define TOKEN_CONTINUE			HAZE_TEXT("继续")
#define TOKEN_RETURN			HAZE_TEXT("返回")

#define TOKEN_WHILE				HAZE_TEXT("当")

#define TOKEN_CAST				HAZE_TEXT("转")

#define TOKEN_REFERENCE			HAZE_TEXT("引用")

#define TOKEN_DEFINE			HAZE_TEXT("定义")

#define TOKEN_IMPORT_MODULE		HAZE_TEXT("引")

static std::unordered_map<std::wstring, HazeToken> TokenMap =
{
	{TOKEN_BOOL, HazeToken::Bool},
	{TOKEN_CHAR, HazeToken::Char},
	{TOKEN_BYTE, HazeToken::Byte},
	{TOKEN_SHORT, HazeToken::Short},
	{TOKEN_INT, HazeToken::Int},
	{TOKEN_FLOAT, HazeToken::Float},
	{TOKEN_LONG, HazeToken::Long},
	{TOKEN_DOUBLE, HazeToken::Double},

	{TOKEN_UNSIGN_BYTE, HazeToken::UnsignByte},
	{TOKEN_UNSIGN_SHORT, HazeToken::UnsignShort},
	{TOKEN_UNSIGN_INT, HazeToken::UnsignInt},
	{TOKEN_UNSIGN_LONG, HazeToken::UnsignLong},

	{TOKEN_CLASS, HazeToken::Class},
	{TOKEN_CLASS_DATA, HazeToken::ClassData},
	{TOKEN_CLASS_FUNCTION, HazeToken::ClassFunction},

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

	{TOKEN_LEFT_PARENTHESES, HazeToken::LeftParentheses},
	{TOKEN_RIGHT_PARENTHESES, HazeToken::RightParentheses},
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

	{TOKEN_IMPORT_MODULE, HazeToken::ImportModule},
};

Parse::Parse(HazeVM* VM) :VM(VM)
{
}

Parse::~Parse()
{
}

void Parse::InitializeFile(const std::wstring& FilePath)
{
	std::wifstream FS(FilePath);
	FS.imbue(std::locale("chs"));
	std::wstring Content(std::istreambuf_iterator<wchar_t>(FS), {});
	CodeText = std::move(Content);
	FS.close();
}

void Parse::InitializeString(const std::wstring& String)
{
	CodeText = String;
}

void Parse::ParseContent()
{
	HazeToken Token;
	while (!TokenIsNone(Token = GetNextToken()))
	{
		switch (Token)
		{
		case HazeToken::None:
			break;
		case HazeToken::Bool:
		case HazeToken::Char:
		case HazeToken::Byte:
		case HazeToken::Short:
		case HazeToken::Int:
		case HazeToken::Float:
		case HazeToken::Long:
		case HazeToken::Double:
		case HazeToken::UnsignByte:
		case HazeToken::UnsignShort:
		case HazeToken::UnsignInt:
		case HazeToken::UnsignLong:
			ParseExpression();
			break;
		case HazeToken::Class:
			break;
		case HazeToken::ClassData:
			break;
		case HazeToken::ClassFunction:
			break;
		case HazeToken::ImportModule:
			break;
		case HazeToken::Identifier:
			ParseUnaryExpression();
			break;

		default:
			break;
		}
	}
}

HazeToken Parse::GetNextToken()
{
	const wchar_t* Code = CodeText.c_str();
	if (!*Code)
	{
		return HazeToken::None;
	}

	while (isspace(*Code))
	{
		Code++;
	}

	//Match Token
	CurrLexeme.clear();
	while (!isspace(*Code))
	{
		CurrLexeme += *(Code++);
	}

	auto It = TokenMap.find(CurrLexeme);
	if (It != TokenMap.end())
	{
		CurrToken = It->second;
	}
	else
	{
		CurrToken = HazeToken::Identifier;
	}

	return CurrToken;
}

void Parse::HandleParseExpression()
{
	ParseExpression();
}

std::unique_ptr<ASTBase> Parse::ParseExpression()
{
	std::unique_ptr<ASTBase> Left = ParseUnaryExpression();
	if (Left)
	{
		return ParseBinaryOperateExpression();
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseUnaryExpression()
{
	/*HazeToken TypeToken = Token;
	std::wstring Name = L"";

	if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("Parse expression except name error\n")))
	{
		Name = CurrLexeme;
	}

	HazeToken ExpressionToken = GetNextToken();
	if (ExpressionToken == HazeToken::Assign)
	{
		//赋值
		ParseUnaryExpression();
	}
	else if (ExpressionToken == HazeToken::LeftParentheses)
	{
		//函数
	}
	else
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("Parse expression error\n"));
	}*/
}

std::unique_ptr<ASTBase> Parse::ParseBinaryOperateExpression()
{
	return std::unique_ptr<ASTBase>();
}

std::unique_ptr<ASTBase> Parse::ParsePrimary()
{
	HazeToken Token = CurrToken;
	switch (Token)
	{
	case HazeToken::Bool:
	case HazeToken::Char:
	case HazeToken::Byte:
	case HazeToken::Short:
	case HazeToken::Int:
	case HazeToken::Float:
	case HazeToken::Long:
	case HazeToken::Double:
	case HazeToken::UnsignByte:
	case HazeToken::UnsignShort:
	case HazeToken::UnsignInt:
	case HazeToken::UnsignLong:
		return ParseVariableDefine();
	default:
		break;
	}
	return std::unique_ptr<ASTBase>();
}

std::unique_ptr<ASTBase> Parse::ParseVariableDefine()
{
	HazeToken TypeToken = CurrToken;
	if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("Error: Parse bool expression name wrong\n")))
	{
		std::wstring VariableName = CurrLexeme;

		if (ExpectNextTokenIs(HazeToken::Assign, HAZE_TEXT("Error: Parse bool expression expect = \n")))
		{
			std::unique_ptr<ASTBase> Expression = ParseExpression();
		}
	}

	return std::unique_ptr<ASTBase>();
}

std::unique_ptr<ASTBase> Parse::ParseBoolExpression()
{
	return nullptr;
}

bool Parse::ExpectNextTokenIs(HazeToken Token, const wchar_t* ErrorInfo)
{
	HazeToken NextToken = GetNextToken();
	if (Token != NextToken)
	{
		HazeLog::LogInfo(HazeLog::Error, ErrorInfo);
		return false;
	}

	return true;
}