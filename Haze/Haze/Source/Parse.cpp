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
#include <cstdarg>

#include "HazeTokenText.h"

//因为会出现嵌套解析同一种AST的情况，所以需要记录不同的行

static std::unordered_map<HAZE_STRING, HazeToken> HashMap_Token =
{
	{TOKEN_VOID, HazeToken::Void},
	{TOKEN_BOOL, HazeToken::Bool},
	{TOKEN_BYTE, HazeToken::Byte},
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

	{TOKEN_BIT_AND, HazeToken::BitAnd},
	{TOKEN_BIT_OR, HazeToken::BitOr},
	{TOKEN_BIT_NEG, HazeToken::BitNeg},
	{TOKEN_BIT_XOR, HazeToken::BitXor},

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
	{TOKEN_BIT_AND_ASSIGN, HazeToken::BitAndAssign},
	{TOKEN_BIT_OR_ASSIGN, HazeToken::BitOrAssign},
	{TOKEN_BIT_XOR_ASSIGN, HazeToken::BitXorAssign},

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

	{TOKEN_DEFINE, HazeToken::Define},

	{TOKEN_STANDARD_LIBRARY, HazeToken::StandardLibrary},
	{TOKEN_IMPORT_MODULE, HazeToken::ImportModule},

	{TOKEN_MULTI_VARIABLE, HazeToken::MultiVariable},

	{TOKEN_NEW, HazeToken::New},

	{TOKEN_QUESTION_MARK, HazeToken::ThreeOperatorStart},
	{TOKEN_QUESTIOB_COLON, HazeToken::ThreeOperatorBranch},

	{TOKEN_NULL_PTR, HazeToken::NullPtr},
};

const std::unordered_map<HAZE_STRING, HazeToken>& GetHashMap_Token()
{
	return HashMap_Token;
}

static std::unordered_map<HazeToken, int> HashMap_OperatorPriority =
{
	{ HazeToken::Assign, 1000 },
	{ HazeToken::AddAssign, 1000 },
	{ HazeToken::SubAssign, 1000 },
	{ HazeToken::MulAssign, 1000 },
	{ HazeToken::DivAssign, 1000 },
	{ HazeToken::ModAssign, 1000 },
	{ HazeToken::BitAndAssign, 1000 },
	{ HazeToken::BitOrAssign, 1000 },
	{ HazeToken::BitXorAssign, 1000 },
	{ HazeToken::ShlAssign, 1000 },
	{ HazeToken::ShrAssign, 1000 },

	{ HazeToken::ThreeOperatorStart, 1200 },

	{ HazeToken::Or, 1400 },
	{ HazeToken::And, 1500 },

	{ HazeToken::BitOr, 1700 },
	{ HazeToken::BitXor, 1800 },
	{ HazeToken::BitAnd, 1900 },

	{ HazeToken::Equal, 2000 },
	{ HazeToken::NotEqual, 2000 },

	{ HazeToken::Greater, 2500 },
	{ HazeToken::GreaterEqual, 2500 },
	{ HazeToken::Less, 2500 },
	{ HazeToken::LessEqual, 2500 },

	{ HazeToken::Shl, 3000 },
	{ HazeToken::Shr, 3000 },


	{ HazeToken::Add, 4000 },
	{ HazeToken::Sub, 4000 },

	{ HazeToken::Mul, 5000 },
	{ HazeToken::Div, 5000 },
	{ HazeToken::Mod, 5000 },

	//{ HazeToken::BitNeg, 6000 }

	{ HazeToken::PointerValue, 7000 },
	{ HazeToken::GetAddress, 7000 },

	//{ HazeToken::Not, 6000 },
	//{ HazeToken::Inc, 9000 },				//后置++

	
	//{ HazeToken::LeftParentheses, 10000 },	
};

static HazeValueType GetPointerBaseType(const HAZE_STRING& Str)
{
	HazeToken Token = HashMap_Token.find(Str.substr(0, Str.length() - 1))->second;
	return GetValueTypeByToken(Token);
}

static HAZE_STRING GetPointerClassType(const HAZE_STRING& Str)
{
	return Str.substr(0, Str.length() - 1);
}

static void GetType(HazeDefineType& Type, const HAZE_STRING& Str)
{
	if (Type.PrimaryType == HazeValueType::PointerBase)
	{
		Type.SecondaryType = GetPointerBaseType(Str);
	}
	else if (Type.PrimaryType == HazeValueType::PointerClass)
	{
		Type.CustomName = GetPointerClassType(Str);
	}
	else if (Type.PrimaryType == HazeValueType::Class)
	{
		Type.CustomName = Str;
	}
}

Parse::Parse(HazeCompiler* Compiler) : Compiler(Compiler), CurrCode(nullptr), CurrToken(HazeToken::None), LeftParenthesesExpressionCount(0),
	LineCount(1), NeedParseNextStatement(false)
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
		case HazeToken::Byte:
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
	bool NewLine = false;
	while (HazeIsSpace(*CurrCode, &NewLine))
	{
		if (NewLine)
		{
			IncLineCount();
		}
		CurrCode++;
	}

	if (HAZE_STRING(CurrCode) == HAZE_TEXT(""))
	{
		IncLineCount();
		CurrToken = HazeToken::None;
		CurrLexeme = HAZE_TEXT("");
		return HazeToken::None;
	}

	//Match Token
	CurrLexeme.clear();
	const HAZE_CHAR* Signal;
	while (!HazeIsSpace(*CurrCode, &NewLine) || CurrToken == HazeToken::StringMatch)
	{
		if (NewLine)
		{
			IncLineCount(true);
		}

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
				if (CurrPreLexeme.first.empty())
				{
					CurrPreLexeme.first = CurrLexeme;
					CurrPreLexeme.second = 3;
					CurrLexeme = Signal;
					CurrCode += 3;
				}
			}
			else if (IsHazeSignalToken(CurrCode, Signal, 2))
			{
				if (CurrPreLexeme.first.empty())
				{
					CurrPreLexeme.first = CurrLexeme;
					CurrPreLexeme.second = 2;
					CurrLexeme = Signal;
					CurrCode += 2;
				}
			}
			else if (CurrLexeme.length() == 0 || IsPointerOrRef(CurrLexeme + *CurrCode, CurrToken)
				|| IsHazeSignalToken(CurrCode - 1, Signal, 2))
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
		
			HazeIsSpace(*CurrCode, &NewLine);
			if (NewLine)
			{
				IncLineCount();
			}
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
	else if (IsPointerOrRef(CurrLexeme, CurrToken))
	{
	}
	else if (Compiler->IsClass(CurrLexeme) || CurrParseClass == CurrLexeme)
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

std::unique_ptr<ASTBase> Parse::ParseExpression(int Prec)
{
	if (NeedParseNextStatement)
	{
		NeedParseNextStatement = false;
	}

	std::unique_ptr<ASTBase> Left = ParseUnaryExpression();

	if (Left)
	{
		return ParseBinaryOperateExpression(Prec, std::move(Left));
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseUnaryExpression()
{
	auto it = HashMap_OperatorPriority.find(CurrToken);
	if (it == HashMap_OperatorPriority.end())
	{
		return ParsePrimary();
	}
	else if (CurrToken == HazeToken::Mul)		//给指针指向的值赋值
	{
		return ParsePrimary();
	}
	else if (CurrToken == HazeToken::BitAnd)	//取地址
	{
		return ParseGetAddress();
	}
	else if (CurrToken == HazeToken::Sub)		//取负数
	{
		return ParseNeg();
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseBinaryOperateExpression(int Prec, std::unique_ptr<ASTBase> Left)
{
	uint64 TempLineCount = LineCount;
	while (true)
	{
		if (NeedParseNextStatement)
		{
			return Left;
		}

		auto it = HashMap_OperatorPriority.find(CurrToken);
		if (it == HashMap_OperatorPriority.end())
		{
			return Left;
		}

		if (it->second < Prec)
		{
			return Left;
		}

		HazeToken OpToken = CurrToken;
		std::unique_ptr<ASTBase> Right = nullptr;
		if (CurrToken == HazeToken::ThreeOperatorStart)
		{
		}
		else
		{
			GetNextToken();
			Right = ParseExpression(it->second);
			if (!Right)
			{
				return nullptr;
			}
		}

		auto NextPrec = HashMap_OperatorPriority.find(CurrToken);
		if (NextPrec == HashMap_OperatorPriority.end())
		{
			return std::make_unique<ASTBinaryExpression>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), OpToken, Left, Right);
		}

		if (it->second < NextPrec->second)
		{
			Right = ParseBinaryOperateExpression(it->second + 1, std::move(Right));
			if (!Right)
			{
				return nullptr;
			}

			Left = std::make_unique<ASTBinaryExpression>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), OpToken, Left, Right);
		}
		else if (NextPrec == HashMap_OperatorPriority.find(HazeToken::ThreeOperatorStart))
		{
			Left = ParseThreeOperator(Right ? std::make_unique<ASTBinaryExpression>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), OpToken, Left, Right) : std::move(Left));
		}
		else
		{
			// Merge LHS/RHS.
			Left = std::make_unique<ASTBinaryExpression>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), OpToken, Left, Right);
		}
	}
}

std::unique_ptr<ASTBase> Parse::ParsePrimary()
{
	HazeToken Token = CurrToken;
	switch (Token)
	{
	case HazeToken::Bool:
	case HazeToken::Byte:
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
	case HazeToken::ReferenceBase:
	case HazeToken::ReferenceClass:
	case HazeToken::PointerFunction:
	case HazeToken::PointerPointer:
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
	case HazeToken::Break:
		return ParseBreakExpression();
	case HazeToken::Continue:
		return ParseContinueExpression();
	case HazeToken::Return:
		return ParseReturn();
	case HazeToken::Inc:
		return ParseInc();
	case HazeToken::Dec:
		return ParseDec();
	case HazeToken::New:
		return ParseNew();
	case HazeToken::LeftParentheses:
		return ParseLeftParentheses();
	case HazeToken::LeftBrace:
		return ParseLeftBrace();
	case HazeToken::Mul:	//pointer value
		return ParsePointerValue();
	case HazeToken::BitAnd:
		return ParseGetAddress();
	case HazeToken::BitNeg:
		return ParseNeg();
	case HazeToken::NullPtr:
		return ParseNullPtr();
	default:
		break;
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseIdentifer()
{
	uint64 TempLineCount = LineCount;
	std::unique_ptr<ASTBase> Ret = nullptr;
	HAZE_STRING IdentiferName = CurrLexeme;
	std::vector<std::unique_ptr<ASTBase>> IndexExpression;

	if (GetNextToken() == HazeToken::LeftParentheses && LineCount == TempLineCount)
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
		return std::make_unique<ASTFunctionCall>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), IdentiferName, VectorParam);
	}
	else if (CurrToken == HazeToken::Array)
	{
		while (CurrToken == HazeToken::Array)
		{
			GetNextToken();
			IndexExpression.push_back(ParseExpression());
			
			GetNextToken();
		}

		Ret = std::make_unique<ASTIdentifier>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), IdentiferName, IndexExpression);
	}
	else
	{
		Ret = std::make_unique<ASTIdentifier>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), IdentiferName, IndexExpression);
	}

	return Ret;
}

std::unique_ptr<ASTBase> Parse::ParseVariableDefine()
{
	uint64 TempLineCount = LineCount;
	DefineVariable.Type.PrimaryType = GetValueTypeByToken(CurrToken);

	int PointerLevel = 1;
	if (CurrToken == HazeToken::CustomClass)
	{
		DefineVariable.Type.CustomName = CurrLexeme;
	}
	else if (CurrToken == HazeToken::PointerBase || CurrToken == HazeToken::ReferenceBase)
	{
		DefineVariable.Type.SecondaryType = GetPointerBaseType(CurrLexeme);
		DefineVariable.Type.CustomName = HAZE_TEXT("");
	}
	else if (CurrToken == HazeToken::PointerClass || CurrToken == HazeToken::ReferenceClass)
	{
		DefineVariable.Type.SecondaryType = HazeValueType::Class;
		DefineVariable.Type.CustomName = GetPointerClassType(CurrLexeme);
	}

	if (CurrToken == HazeToken::PointerBase || CurrToken == HazeToken::PointerClass)
	{
		while (GetNextToken() == HazeToken::Mul)
		{
			PointerLevel += 1;
		}

		if (PointerLevel > 1)
		{
			HAZE_TO_DO(Parse pointer pointer !);
		}
	}
	else
	{
		GetNextToken();
	}

	std::vector<std::unique_ptr<ASTBase>> ArraySize;
	if (CurrToken == HazeToken::Identifier)
	{
		DefineVariable.Name = CurrLexeme;

		GetNextToken();

		if (CurrToken == HazeToken::Array)
		{
			DefineVariable.Type.SecondaryType = DefineVariable.Type.PrimaryType;
			DefineVariable.Type.PrimaryType = HazeValueType::Array;

			while (CurrToken == HazeToken::Array)
			{
				GetNextToken();
				ArraySize.push_back(ParseExpression());
				GetNextToken();
			}
		}

		if (CurrToken == HazeToken::Assign)
		{
			GetNextToken();		//吃掉赋值符号
			std::unique_ptr<ASTBase> Expression = ParseExpression();

			return std::make_unique<ASTVariableDefine>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), DefineVariable, std::move(Expression), std::move(ArraySize), PointerLevel);
		}
		else if ((CurrToken == HazeToken::RightParentheses || CurrToken == HazeToken::Comma) && StackSectionSignal.top() == HazeSectionSignal::Function)
		{
			//函数调用
			return std::make_unique<ASTVariableDefine>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), DefineVariable, nullptr);
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
					return std::make_unique<ASTVariableDefine>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), DefineVariable, nullptr, std::move(ArraySize), PointerLevel);
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
				HAZE_LOG_ERR(HAZE_TEXT("类对象定义需要括号\"(\" !\n"));
			}
		}
		
		else
		{
			return std::make_unique<ASTVariableDefine>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), DefineVariable, nullptr, std::move(ArraySize), PointerLevel);
		}
	}
	else if (CurrToken == HazeToken::LeftParentheses)
	{
		//函数指针或数组指针
		if (ExpectNextTokenIs(HazeToken::Mul) && ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("解析错误：函数指针或者数组指针需要一个正确的名称!\n")))
		{
			std::vector<HazeDefineType> Vector_ParamType;
			Vector_ParamType.push_back(DefineVariable.Type);

			DefineVariable.Name = CurrLexeme;
			
			if (ExpectNextTokenIs(HazeToken::RightParentheses))
			{
				if (ExpectNextTokenIs(HazeToken::LeftParentheses))
				{
					DefineVariable.Type.PrimaryType = HazeValueType::PointerFunction;
					GetNextToken();
					while (true)
					{
						HazeDefineType Type;
						Type.PrimaryType = GetValueTypeByToken(CurrToken);
						if (CurrToken == HazeToken::PointerBase)
						{
							Type.SecondaryType = GetPointerBaseType(CurrLexeme);
						}
						else if (CurrToken == HazeToken::PointerClass)
						{
							Type.CustomName = GetPointerClassType(CurrLexeme);
						}
						else if (CurrToken == HazeToken::Class)
						{
							Type.CustomName = CurrLexeme;
						}

						Vector_ParamType.push_back(Type);

						if (ExpectNextTokenIs(HazeToken::RightParentheses))
						{
							break;
						}
						else if (CurrToken == HazeToken::Comma)
						{
							GetNextToken();
						}
					}

					if (ExpectNextTokenIs(HazeToken::Assign))
					{
						GetNextToken();

						std::unique_ptr<ASTBase> Expression = ParseExpression();

						return std::make_unique<ASTVariableDefine>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), DefineVariable, std::move(Expression), std::move(ArraySize), PointerLevel);
					}
				}
				else if (CurrToken == HazeToken::Array)
				{
					DefineVariable.Type.PrimaryType = HazeValueType::PointerArray;

					GetNextToken();
					while (true)
					{
						ArraySize.push_back(ParseExpression());
						
						if (!ExpectNextTokenIs(HazeToken::Array))
						{
							break;
						}
					}

					if (CurrToken == HazeToken::Assign)
					{
						GetNextToken();

						std::unique_ptr<ASTBase> Expression = ParseExpression();

						return std::make_unique<ASTVariableDefine>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), DefineVariable, std::move(Expression), std::move(ArraySize), PointerLevel);
					}
				}
			}
		}
	}

	return std::make_unique<ASTVariableDefine>(Compiler, SourceLocation(LineCount), StackSectionSignal.top(), DefineVariable, nullptr, std::move(ArraySize), PointerLevel);;
}

std::unique_ptr<ASTBase> Parse::ParseStringText()
{
	uint64 TempLineCount = LineCount;
	GetNextToken();
	HAZE_STRING Text = CurrLexeme;

	GetNextToken();
	return std::make_unique<ASTStringText>(Compiler, SourceLocation(TempLineCount), Text);
}

std::unique_ptr<ASTBase> Parse::ParseBoolExpression()
{
	uint64 TempLineCount = LineCount;
	HazeValue Value;
	Value.Value.Bool = CurrLexeme == TOKEN_TRUE;

	GetNextToken();
	return std::make_unique<ASTBool>(Compiler, SourceLocation(TempLineCount), Value);
}

std::unique_ptr<ASTBase> Parse::ParseNumberExpression()
{
	uint64 TempLineCount = LineCount;
	HazeValue Value;
	HazeValueType Type = GetNumberDefaultType(CurrLexeme);

	StringToHazeValueNumber(CurrLexeme, Type, Value);

	GetNextToken();
	return std::make_unique<ASTNumber>(Compiler, SourceLocation(TempLineCount), Type, Value);
}

std::unique_ptr<ASTBase> Parse::ParseIfExpression(bool Recursion)
{
	uint64 TempLineCount = LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("解析错误：若 表达式期望捕捉 (\n")))
	{
		GetNextToken();
		auto ConditionExpression = ParseExpression();

		std::unique_ptr<ASTBase> IfMultiExpression = nullptr;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("解析错误：若 执行表达式期望捕捉 { \n")))
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

		return std::make_unique<ASTIfExpression>(Compiler, SourceLocation(TempLineCount), ConditionExpression, IfMultiExpression, ElseExpression);
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseForExpression()
{
	uint64 TempLineCount = LineCount;
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
			return std::make_unique<ASTForExpression>(Compiler, SourceLocation(TempLineCount), InitExpression, ConditionExpression, StepExpression, MultiExpression);
		}
	}
	
	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseWhileExpression()
{
	uint64 TempLineCount = LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("解析错误：当 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto ConditionExpression = ParseExpression();

		std::unique_ptr<ASTBase> MultiExpression = nullptr;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("解析错误：当 执行表达式期望捕捉 {")))
		{
			MultiExpression = ParseMultiExpression();
			
			GetNextToken();
			return std::make_unique<ASTWhileExpression>(Compiler, SourceLocation(TempLineCount), ConditionExpression, MultiExpression);
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseBreakExpression()
{
	uint64 TempLineCount = LineCount;
	GetNextToken();
	return std::make_unique<ASTBreakExpression>(Compiler, SourceLocation(TempLineCount));
}

std::unique_ptr<ASTBase> Parse::ParseContinueExpression()
{
	uint64 TempLineCount = LineCount;
	GetNextToken();
	return std::make_unique<ASTContinueExpression>(Compiler, SourceLocation(TempLineCount));
}

std::unique_ptr<ASTBase> Parse::ParseReturn()
{
	uint64 TempLineCount = LineCount;
	GetNextToken();
	auto ReturnExpression = ParseExpression();
	return std::make_unique<ASTReturn>(Compiler, SourceLocation(TempLineCount), ReturnExpression);
}

std::unique_ptr<ASTBase> Parse::ParseNew()
{
	uint64 TempLineCount = LineCount;
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
			return std::make_unique<ASTNew>(Compiler, SourceLocation(TempLineCount), Define);
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseInc()
{
	uint64 TempLineCount = LineCount;
	bool IsPre = CurrPreLexeme.first.empty();
	std::unique_ptr<ASTBase> Expression;
	if (IsPre)
	{
		GetNextToken();
		Expression = ParseExpression();
	}
	else
	{
		BackToPreLexemeAndNext();
		Expression = ParseExpression();
		GetNextToken();
	}

	return std::make_unique<ASTInc>(Compiler, SourceLocation(TempLineCount), Expression, IsPre);
}

std::unique_ptr<ASTBase> Parse::ParseDec()
{
	uint64 TempLineCount = LineCount;
	bool IsPre = CurrPreLexeme.first.empty();
	std::unique_ptr<ASTBase> Expression;
	if (IsPre)
	{
		GetNextToken();
		Expression = ParseExpression();
	}
	else
	{
		BackToPreLexemeAndNext();
		Expression = ParseExpression();
		GetNextToken();
	}

	return std::make_unique<ASTDec>(Compiler, SourceLocation(TempLineCount), Expression, IsPre);
}

std::unique_ptr<ASTBase> Parse::ParseThreeOperator(std::unique_ptr<ASTBase> Condition)
{
	uint64 TempLineCount = LineCount;
	auto Iter = HashMap_OperatorPriority.find(HazeToken::ThreeOperatorStart);
	if (Iter != HashMap_OperatorPriority.end())
	{
		auto ConditionExpression = std::move(Condition);

		GetNextToken();
		auto LeftExpression = ParseExpression(Iter->second);

		if (CurrToken != HazeToken::ThreeOperatorBranch)
		{
			HAZE_LOG_ERR(HAZE_TEXT("三目表达式 需要 : 符号!\n"));
			return nullptr;
		}

		GetNextToken();
		auto RightExpression = ParseExpression(Iter->second);
	
		return std::make_unique<ASTThreeExpression>(Compiler, SourceLocation(TempLineCount), ConditionExpression, LeftExpression, RightExpression);
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseLeftParentheses()
{
	LeftParenthesesExpressionCount++;
	GetNextToken();
	auto Expression = ParseExpression();
	if (CurrToken == HazeToken::RightParentheses || LeftParenthesesExpressionCount == 0)
	{
		if (LeftParenthesesExpressionCount > 0)
		{
			LeftParenthesesExpressionCount--;
			GetNextToken();
		}
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
	uint64 TempLineCount = LineCount;
	int Level = (int)CurrLexeme.length();

	GetNextToken();
	
	auto Expression = ParseExpression(HashMap_OperatorPriority.find(HazeToken::PointerValue)->second);

	if (CurrToken == HazeToken::RightParentheses)
	{
		if (LeftParenthesesExpressionCount > 0)
		{
			LeftParenthesesExpressionCount--;
			GetNextToken();
			if (CurrToken == HazeToken::Assign)
			{
				GetNextToken();
				auto AssignExpression = ParseExpression();
				return std::make_unique<ASTPointerValue>(Compiler, SourceLocation(TempLineCount), Expression, Level, std::move(AssignExpression));
			}
		}

		return std::make_unique<ASTPointerValue>(Compiler, SourceLocation(TempLineCount), Expression, Level);
	}
	else if (CurrToken == HazeToken::Assign)
	{
		GetNextToken();
		auto AssignExpression = ParseExpression();
		return std::make_unique<ASTPointerValue>(Compiler, SourceLocation(TempLineCount), Expression, Level, std::move(AssignExpression));
	}
	else 
	{
		return std::make_unique<ASTPointerValue>(Compiler, SourceLocation(TempLineCount), Expression, Level);
	}
}

std::unique_ptr<ASTBase> Parse::ParseNeg()
{
	uint64 TempLineCount = LineCount;
	bool IsNumberNeg = CurrToken == HazeToken::Sub;
	GetNextToken();
	auto Expression = ParseExpression();
	
	return std::make_unique<ASTNeg>(Compiler, SourceLocation(TempLineCount), Expression, IsNumberNeg);
}

std::unique_ptr<ASTBase> Parse::ParseNullPtr()
{
	uint64 TempLineCount = LineCount;
	GetNextToken();
	return std::make_unique<ASTNullPtr>(Compiler, SourceLocation(TempLineCount));
}

std::unique_ptr<ASTBase> Parse::ParseGetAddress()
{
	uint64 TempLineCount = LineCount;
	GetNextToken();
	auto Expression = ParseExpression(HashMap_OperatorPriority.find(HazeToken::GetAddress)->second);
	NeedParseNextStatement = TempLineCount != LineCount;

	return std::make_unique<ASTGetAddress>(Compiler, SourceLocation(TempLineCount), Expression);
}

std::unique_ptr<ASTBase> Parse::ParseLeftBrace()
{
	uint64 TempLineCount = LineCount;
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

	return std::make_unique<ASTInitializeList>(Compiler, SourceLocation(TempLineCount), Vector_Element);
}

//std::unique_ptr<ASTBase> Parse::ParseOperatorAssign()
//{
//	HazeToken Token = CurrToken;
//
//	auto Value = ParseExpression();
//
//	GetNextToken();
//	return std::make_unique<ASTOperetorAssign>(Compiler, SourceLocation(LineCount) Token, Value);
//}

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

	return std::make_unique<ASTMultiExpression>(Compiler, SourceLocation(LineCount), StackSectionSignal.top(), VectorExpr);
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
		return std::make_unique<ASTFunctionSection>(Compiler, SourceLocation(LineCount), Functions);
	}

	return nullptr;
}

std::unique_ptr<ASTFunction> Parse::ParseFunction(const HAZE_STRING* ClassName)
{
	uint64 TempLineCount = LineCount;
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
				GetType(Param.Type, CurrLexeme);

				if (Param.Type.PrimaryType == HazeValueType::MultiVariable)
				{
					Param.Name = HAZE_MULTI_PARAM_NAME;
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
					return std::make_unique<ASTFunction>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), FunctionName, FuncType, Vector_Param, Body);
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
					Param.Name = HAZE_MULTI_PARAM_NAME;
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
					return std::make_unique<ASTFunction>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), FunctionName, FuncType, Vector_Param, Body);
				}
			}
		}
	}

	return nullptr;
}

std::unique_ptr<ASTFunction> Parse::ParseMainFunction()
{
	uint64 TempLineCount = LineCount;
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
				return std::make_unique<ASTFunction>(Compiler, SourceLocation(TempLineCount), StackSectionSignal.top(), FunctionName, DefineType, Vector_Param, Body);
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

		return std::make_unique<ASTStandardLibrary>(Compiler, SourceLocation(LineCount), StandardLibraryName, Vector_FunctionDefine, Vector_ClassDefine);
	}

	return nullptr;
}

std::unique_ptr<ASTClassDefine> Parse::ParseStandardLibrary_ClassDefine()
{
	CurrParseClass = CurrLexeme;
	CurrParseClass.clear();
	/*std::vector<std::vector<std::unique_ptr<ASTBase>>> Data;
	std::vector<std::unique_ptr<ASTFunctionDefine>> Function;
	return std::make_unique<ASTClassDefine>(Compiler, SourceLocation(LineCount) CurrLexeme, Data, Function);*/
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
			if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("解析错误: \n")))
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
							Param.Type.SecondaryType = GetPointerBaseType(CurrLexeme);
						}
						else if (CurrToken == HazeToken::PointerClass)
						{
							Param.Type.CustomName = GetPointerClassType(CurrLexeme);
						}

						if (Param.Type.PrimaryType == HazeValueType::MultiVariable)
						{
							Param.Name = HAZE_MULTI_PARAM_NAME;
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
					Vector_FunctionDefine.push_back(std::make_unique<ASTFunctionDefine>(Compiler, SourceLocation(LineCount), FunctionName, FuncType, Vector_Param));

				}
			}
		}

		ExpectNextTokenIs(HazeToken::RightBrace, HAZE_TEXT("Error: Parse standard library end need } \n"));

		GetNextToken();

		//return std::make_unique<ASTStandardLibrary>(Compiler, SourceLocation(LineCount) StandardLibraryName, Vector_FunctionDefine);
	}

	return Vector_FunctionDefine;
}

std::unique_ptr<ASTBase> Parse::ParseImportModule()
{
	if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("Parse import module name is error\n")))
	{
		HAZE_STRING Name = CurrLexeme;
		GetNextToken();
		return std::make_unique<ASTImportModule>(Compiler, SourceLocation(LineCount), Name);
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
			return std::make_unique<ASTClass>(Compiler, SourceLocation(LineCount), Name, Vector_ClassData, ClassFunction);
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
					GetNextToken();
					while (CurrToken != HazeToken::RightBrace)
					{
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
					GetNextToken();
					while (CurrToken != HazeToken::RightBrace)
					{
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
		return std::make_unique<ASTClassFunctionSection>(Compiler, SourceLocation(LineCount), Vector_ClassFunction);
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
		TOKEN_BIT_AND, TOKEN_BIT_OR, TOKEN_BIT_NEG, TOKEN_BIT_XOR,
		TOKEN_EQUAL, TOKEN_NOT_EQUAL, TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL,
		TOKEN_LEFT_PARENTHESES, TOKEN_RIGHT_PARENTHESES, 
		TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, 
		HAZE_SINGLE_COMMENT, HAZE_MULTI_COMMENT_START, HAZE_MULTI_COMMENT_END,
		TOKEN_COMMA, 
		TOKEN_MULTI_VARIABLE, 
		TOKEN_STRING_MATCH,
		TOKEN_ARRAY_START, TOKEN_ARRAY_END,
		TOKEN_INC, TOKEN_DEC, TOKEN_ADD_ASSIGN, TOKEN_SUB_ASSIGN, TOKEN_MUL_ASSIGN, TOKEN_DIV_ASSIGN, TOKEN_MOD_ASSIGN, TOKEN_SHL_ASSIGN, TOKEN_SHR_ASSIGN,
		TOKEN_BIT_AND_ASSIGN, TOKEN_BIT_OR_ASSIGN, TOKEN_BIT_XOR_ASSIGN,
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

bool Parse::IsPointerOrRef(const HAZE_STRING& Str, HazeToken& OutToken)
{
	HAZE_STRING TypeName = Str.substr(0, Str.length() - 1);
	HAZE_STRING PointerChar = Str.substr(Str.length() - 1, 1);

	if (PointerChar == TOKEN_MUL)
	{
		auto It = HashMap_Token.find(TypeName);
		if (It != HashMap_Token.end())
		{
			if (IsHazeDefaultTypeAndVoid(GetValueTypeByToken(It->second)))
			{
				OutToken = HazeToken::PointerBase;
				return true;
			}
		}
		else
		{
			if (Compiler->IsClass(TypeName))
			{
				OutToken = HazeToken::PointerClass;
				return true;
			}

			for (size_t i = 0; i < Str.length() - 1; i++)
			{
				if (Str.substr(i, 1) != TOKEN_MUL) 
				{
					return false;
				}
			}

			if (Str.length() > 3)
			{
				HAZE_LOG_ERR(HAZE_TEXT("Haze max 3 level pointer pointer !\n"));
				return false;
			}

			OutToken = HazeToken::PointerPointer;
			return true;
		}
	}
	else if (PointerChar == TOKEN_BIT_AND)
	{
		auto It = HashMap_Token.find(TypeName);
		if (It != HashMap_Token.end())
		{
			if (IsHazeDefaultTypeAndVoid(GetValueTypeByToken(It->second)))
			{
				OutToken = HazeToken::ReferenceBase;
				return true;
			}
		}
		else
		{
			if (Compiler->IsClass(TypeName))
			{
				OutToken = HazeToken::ReferenceClass;
				return true;
			}
		}
	}

	return false;
}

void Parse::BackToPreLexemeAndNext()
{
	if (!CurrPreLexeme.first.empty())
	{
		CurrCode -= CurrPreLexeme.second + CurrPreLexeme.first.length();
		GetNextToken();
		CurrPreLexeme.first.clear();
		CurrPreLexeme.second = 0;
	}
}

void Parse::IncLineCount(bool Insert)
{
	LineCount++;

//#if HAZE_DEBUG_ENABLE
//
//	if (Insert)
//	{
//		Compiler->InsertLineCount(LineCount);
//	}
//
//#endif // HAZE_DEBUG_ENABLE

}
