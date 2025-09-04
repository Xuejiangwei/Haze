#include "HazePch.h"
#include "HazeVM.h"
#include "Parse.h"

#include "ASTBase.h"
#include "ASTClass.h"
#include "ASTEnum.h"
#include "ASTFunction.h"
#include "ASTLibrary.h"

#include "Compiler.h"
#include "CompilerSymbol.h"
#include "CompilerHelper.h"
#include "CompilerModule.h"

#include "HazeLogDefine.h"
#include <cstdarg>

#include "HazeTokenText.h"


//因为会出现嵌套解析同一种AST的情况，所以需要记录不同的行
static HashMap<STDString, HazeToken> s_HashMap_Token =
{
	{ TOKEN_VOID, HazeToken::Void },
	{ TOKEN_BOOL, HazeToken::Bool },
	{ TOKEN_INT, HazeToken::Int32 },
	{ TOKEN_UNSIGNED_INT, HazeToken::UInt32 },
	{ TOKEN_FLOAT, HazeToken::Float32 },

	{ TOKEN_ARRAY_START, HazeToken::Array },
	{ TOKEN_ARRAY_END, HazeToken::ArrayDefineEnd },

	{ TOKEN_STRING, HazeToken::String },
	{ TOKEN_STRING_MATCH, HazeToken::StringMatch },

	{ TOKEN_DATA, HazeToken::Data },
	{ TOKEN_FUNCTION, HazeToken::Function },

	{ TOKEN_ENUM, HazeToken::Enum },

	{ TOKEN_CLASS, HazeToken::Class },
	{ TOKEN_CLASS_DATA_PUBLIC, HazeToken::ClassPublic },
	{ TOKEN_CLASS_DATA_PRIVATE, HazeToken::ClassPrivate },

	{ TOKEN_DYNAMIC_CLASS, HazeToken::DynamicClass },

	{ TOKEN_THIS, HazeToken::This },
	{ TOKEN_CLASS_ATTR, HazeToken::ClassAttr },

	{ TOKEN_TRUE, HazeToken::True },
	{ TOKEN_FALSE, HazeToken::False },

	{ TOKEN_ADD, HazeToken::Add },
	{ TOKEN_SUB, HazeToken::Sub },
	{ TOKEN_MUL, HazeToken::Mul },
	{ TOKEN_DIV, HazeToken::Div },
	{ TOKEN_MOD, HazeToken::Mod },

	{ TOKEN_AND, HazeToken::And },
	{ TOKEN_OR, HazeToken::Or },
	{ TOKEN_NOT, HazeToken::Not },

	{ TOKEN_BIT_AND, HazeToken::BitAnd },
	{ TOKEN_BIT_OR, HazeToken::BitOr },
	{ TOKEN_BIT_NEG, HazeToken::BitNeg },
	{ TOKEN_BIT_XOR, HazeToken::BitXor },

	{ TOKEN_LEFT_MOVE, HazeToken::Shl },
	{ TOKEN_RIGHT_MOVE, HazeToken::Shr },

	{ TOKEN_ASSIGN, HazeToken::Assign },
	{ TOKEN_EQUAL, HazeToken::Equal },
	{ TOKEN_NOT_EQUAL, HazeToken::NotEqual },
	{ TOKEN_GREATER, HazeToken::Greater },
	{ TOKEN_GREATER_EQUAL, HazeToken::GreaterEqual },
	{ TOKEN_LESS, HazeToken::Less },
	{ TOKEN_LESS_EQUAL, HazeToken::LessEqual },

	{ TOKEN_INC, HazeToken::Inc },
	{ TOKEN_DEC, HazeToken::Dec },
	{ TOKEN_ADD_ASSIGN, HazeToken::AddAssign },
	{ TOKEN_SUB_ASSIGN, HazeToken::SubAssign },
	{ TOKEN_MUL_ASSIGN, HazeToken::MulAssign },
	{ TOKEN_DIV_ASSIGN, HazeToken::DivAssign },
	{ TOKEN_MOD_ASSIGN, HazeToken::ModAssign },
	{ TOKEN_SHL_ASSIGN, HazeToken::ShlAssign },
	{ TOKEN_SHR_ASSIGN, HazeToken::ShrAssign },
	{ TOKEN_BIT_AND_ASSIGN, HazeToken::BitAndAssign },
	{ TOKEN_BIT_OR_ASSIGN, HazeToken::BitOrAssign },
	{ TOKEN_BIT_XOR_ASSIGN, HazeToken::BitXorAssign },

	{ TOKEN_LEFT_PARENTHESES, HazeToken::LeftParentheses },
	{ TOKEN_RIGHT_PARENTHESES, HazeToken::RightParentheses },

	{ TOKEN_COMMA, HazeToken::Comma },

	{ TOKEN_LEFT_BRACE, HazeToken::LeftBrace },
	{ TOKEN_RIGHT_BRACE, HazeToken::RightBrace },

	{ TOKEN_IF, HazeToken::If },
	{ TOKEN_ELSE, HazeToken::Else },

	{ TOKEN_FOR, HazeToken::For },

	{ TOKEN_BREAK, HazeToken::Break },
	{ TOKEN_CONTINUE, HazeToken::Continue },
	{ TOKEN_RETURN, HazeToken::Return },

	{ TOKEN_WHILE, HazeToken::While },

	{ TOKEN_CAST, HazeToken::Cast },

	{ TOKEN_VIRTUAL, HazeToken::VirtualFunction },
	{ TOKEN_PUREVIRTUAL, HazeToken::PureVirtualFunction },

	//{ TOKEN_STATIC, HazeToken:: },

	{ TOKEN_DEFINE, HazeToken::Define },

	{ TOKEN_STATIC_LIBRARY, HazeToken::StaticLibrary },
	{ TOKEN_DLL_LIBRARY, HazeToken::DLLLibrary },
	{ TOKEN_IMPORT_MODULE, HazeToken::ImportModule },

	{ TOKEN_MULTI_VARIABLE, HazeToken::MultiVariable },

	{ TOKEN_NEW, HazeToken::New },

	{ TOKEN_QUESTION_MARK, HazeToken::ThreeOperatorStart },
	{ TOKEN_QUESTIOB_COLON, HazeToken::Colon },

	{ TOKEN_NULL_PTR, HazeToken::NullPtr },

	{ TOKEN_TYPENAME, HazeToken::TypeName },

	{ TOKEN_SIZE_OF, HazeToken::SizeOf },

	{ TOKEN_TWO_COLON, HazeToken::TwoColon },

	{ TOKEN_GET_ADDRESS, HazeToken::GetAddress },

	{ TOKEN_OBJECT_BASE, HazeToken::ObjectBase },

	{ TOKEN_HASH, HazeToken::Hash },
};

const HashMap<STDString, HazeToken>& GetHashMap_Token()
{
	return s_HashMap_Token;
}

static HashMap<HazeToken, int> s_HashMap_OperatorPriority =
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

	//{ HazeToken::Not, 7000 },
	{ HazeToken::Inc, 9000 },
	{ HazeToken::Dec, 9000 },

	//{ HazeToken::LeftParentheses, 10000 },
};

static const HashMap<STDString, HazeToken> s_NumberMap = {
		{ TOKEN_INT_8 , HazeToken::Int8 }, { TOKEN_UINT_8 , HazeToken::UInt8 }, { TOKEN_INT_16 , HazeToken::Int16 },
		{ TOKEN_UINT_16 , HazeToken::UInt16 }, { TOKEN_INT_32 , HazeToken::Int32 }, { TOKEN_UINT_32 , HazeToken::UInt32 },
		{ TOKEN_INT_64 , HazeToken::UInt64 }, { TOKEN_UINT_64 , HazeToken::UInt64 },
		{ TOKEN_FLOAT_32 , HazeToken::Float32 }, { TOKEN_FLOAT_64 , HazeToken::Float64 },
};

const HashMap<STDString, HazeToken>& GetHashMap_MoreNumberToken()
{
	return s_NumberMap;
}

bool IsSingleOperator(HazeToken token)
{
	return token == HazeToken::Inc || token == HazeToken::Dec;
}

struct TempCurrCode
{
	TempCurrCode(Parse* parse)
		: Par(parse), CacheCurrCode(parse->m_CurrCode), CacheToken(parse->m_CurrToken), CacheLine(Par->m_LineCount) {}

	void Update()
	{
		CacheCurrCode = Par->m_CurrCode;
		CacheToken = Par->m_CurrToken;
		CacheLine = Par->m_LineCount;
	}

	void Reset()
	{
		Par->m_CurrCode = CacheCurrCode;
		Par->m_CurrToken = CacheToken;
		Par->m_LineCount = CacheLine;
	}

	HazeToken GetCacheToken() const { return CacheToken; }

private:
	Parse* Par;
	const x_HChar* CacheCurrCode;
	HazeToken CacheToken;
	x_uint32 CacheLine;
};

struct ParseStage1Spin
{
	ParseStage1Spin(Parse* parse) : Par(parse)
	{
	}

	~ParseStage1Spin()
	{
		do
		{
			for (;;)
			{
				Par->GetNextToken();
				if (Par->TokenIs(HazeToken::LeftBrace))
				{
					Spin.push_back(1);
					break;
				}
				else if (Par->TokenIs(HazeToken::RightBrace))
				{
					Spin.pop_back();
					break;
				}
			}

		} while (Spin.size() > 0);
	}

private:
	Parse* Par;
	V_Array<int> Spin;
};

struct ExpectNextScope
{
	ExpectNextScope(Parse* parse) : Par(parse)
	{
	}

	~ExpectNextScope()
	{
		if (Par->m_IsExpectIdentiferIsClass_Or_Enum)
		{
			if (Par->TokenIs(HazeToken::Identifier))
			{
				Par->m_CompilerSymbol->RegisterSymbol(Par->m_CurrLexeme);

				// 先将枚举也假设为class
				Par->m_CurrToken = HazeToken::CustomClass;
			}

			Par->m_IsExpectIdentiferIsClass_Or_Enum = false;
		}
	}

private:
	Parse* Par;
};

Parse::Parse(Compiler* compiler)
	: m_Compiler(compiler), m_CurrCode(nullptr), m_CurrToken(HazeToken::None),
	m_LeftParenthesesExpressionCount(0), m_LineCount(1), m_IsParseError(false),
	m_IsParseTemplate(false), m_TemplateTypes(nullptr), m_IsParseArray(false),
	m_IsParseClassData_Or_FunctionParam(false), m_IsExpectIdentiferIsClass_Or_Enum(false)
{
	m_CompilerSymbol = m_Compiler->GetCompilerSymbol();
}

Parse::~Parse()
{
	/*std::ifstream s;
	s.open("");
	std::string s1;
	std::getline(s, s1);*/
}

void Parse::InitializeFile(const STDString& filePath)
{
	m_CodeText = Move(ReadUtf8File(filePath));
	m_CurrCode = m_CodeText.c_str();
	m_ModuleName = GetModuleNameByFilePath(filePath);
}

void Parse::InitializeString(const STDString& str, x_uint32 startLine)
{
	m_CodeText = str;
	m_CurrCode = m_CodeText.c_str();
	m_LineCount = startLine;
	m_ModuleName.resize(0);
}

bool Parse::ParseContent()
{
#define PARSE_STEP_ONE_JMP() { if (m_Compiler->IsStage1()) break; }
#define IS_VALID_AST (m_Compiler->IsStage2() && !m_IsParseError && ast)

	m_StackSectionSignal.push(HazeSectionSignal::Global);

	while (NextTokenNotIs(HazeToken::None))
	{
		switch (m_CurrToken)
		{
			case HazeToken::Bool:
			case HazeToken::Int8:
			case HazeToken::UInt8:
			case HazeToken::Int16:
			case HazeToken::UInt16:
			case HazeToken::Int32:
			case HazeToken::UInt32:
			case HazeToken::Int64:
			case HazeToken::UInt64:
			case HazeToken::Float32:
			case HazeToken::Float64:
			case HazeToken::String:
			case HazeToken::Identifier:
			case HazeToken::CustomClass:
			{
				PARSE_STEP_ONE_JMP();
				auto ast = ParseExpression();
				if (IS_VALID_AST)
				{
					ast->CodeGen(nullptr);
				}
			}
				break;
			case HazeToken::Class:
			{
				auto ast = ParseClass();
				if (IS_VALID_AST)
				{
					ast->CodeGen();
				}
				m_CurrParseClass.clear();
			}
				break;
			case HazeToken::Data:
			{
				auto ast = ParseDataSection();
				if (IS_VALID_AST)
				{
					ast->CodeGen(nullptr);
				}
			}
				break;
			case HazeToken::Function:
			{
				auto cacheCode = m_CurrCode;
				auto ast = ParseFunctionSection();
				if (IS_VALID_AST)
				{
					ast->CodeGen();
				}
				else if (!m_IsParseError && TokenIs(HazeToken::Less))
				{
					//函数指针
					m_CurrCode = cacheCode;
					m_CurrToken = HazeToken::Function;
					auto varAst =  ParseVariableDefine();
					if (IS_VALID_AST)
					{
						varAst->CodeGen(nullptr);
					}
				}
			}
				break;
			case HazeToken::Enum:
			{
				auto ast = ParseEnum();
				if (!m_IsParseError && ast)
				{
					ast->CodeGen();
				}
			}
				break;
			case HazeToken::StaticLibrary:
			case HazeToken::DLLLibrary:
			{
				auto ast = ParseLibrary();
				if (!m_IsParseError && ast)
				{
					ast->CodeGen();
				}
			}
				break;
			case HazeToken::ImportModule:
			{
				auto ast = ParseImportModule();
				if (!m_IsParseError && ast)
				{
					ast->CodeGen(nullptr);
				}
			}
				break;
			default:
				PARSE_ERR_W("未能找到生成相应Token的AST处理");
				return false;
		}

		if (m_IsParseError)
		{
			PARSE_ERR_W("解析错误");
			return false;
		}

		if (m_Compiler->IsCompileError())
		{
			return false;
		}
	}

	m_StackSectionSignal.pop();
	return true;
}

HazeToken Parse::GetNextToken(bool clearLexeme)
{
	

	bool bNewLine = false;
	while (HazeIsSpace(*m_CurrCode, &bNewLine) && m_CurrToken != HazeToken::StringMatch)
	{
		if (bNewLine)
		{
			IncLineCount();
		}
		m_CurrCode++;
	}

	if (HString(m_CurrCode) == H_TEXT(""))
	{
		IncLineCount();
		m_CurrToken = HazeToken::None;
		m_CurrLexeme = H_TEXT("");
		return HazeToken::None;
	}

	//Match Token
	if (clearLexeme)
	{
		m_CurrLexeme.clear();
	}

	ExpectNextScope expectNext(this);
	const x_HChar* signal;
	while (!HazeIsSpace(*m_CurrCode, &bNewLine) || m_CurrToken == HazeToken::StringMatch)
	{
		if (bNewLine)
		{
			IncLineCount();
		}

		if (IsHazeSignalToken(m_CurrCode, signal))
		{
			if (m_CurrToken == HazeToken::StringMatch)
			{
				static STDString tempString;
				tempString = *(m_CurrCode++);
				auto iter = s_HashMap_Token.find(tempString);
				if (iter != s_HashMap_Token.end() && iter->second == HazeToken::StringMatch)
				{
					m_CurrToken = HazeToken::None;
					return m_CurrToken;
				}
				else
				{
					m_CurrLexeme += tempString;
				}
				continue;
			}
			else if (STDString(signal) == TOKEN_ARRAY_START)
			{
				if (m_CurrLexeme.empty())
				{
					m_CurrLexeme += *m_CurrCode++;
					m_CurrToken = HazeToken::Array;
					return m_CurrToken;
				}
			}
			else if (STDString(signal) == TOKEN_BIT_AND)
			{
				if (!m_CurrLexeme.empty())
				{
					m_CurrLexeme += *m_CurrCode++;
					m_CurrToken = HazeToken::Reference;
					return m_CurrToken;
				}
			}
			else if (m_CurrToken == HazeToken::Array && STDString(signal) == TOKEN_ARRAY_END)
			{
				if (m_CurrLexeme.empty())
				{
					m_CurrLexeme += *m_CurrCode++;
				}
				break;
			}
			else if (IsHazeSignalToken(m_CurrCode, signal, 3))
			{
				if (m_CurrLexeme.empty())
				{
					m_CurrLexeme = signal;
					m_CurrCode += 3;
				}
			}
			else if (IsHazeSignalToken(m_CurrCode, signal, 2))
			{
				if (m_CurrLexeme.empty())
				{
					m_CurrLexeme = signal;
					m_CurrCode += 2;
				}
			}
			else if (m_CurrLexeme.length() == 0 || IsHazeSignalToken(m_CurrCode - 1, signal, 2))
			{
				m_CurrLexeme += *(m_CurrCode++);
			}
			else if (IsNumber(m_CurrLexeme) && STDString(signal) == TOKEN_CLASS_ATTR)
			{
				m_CurrLexeme += *(m_CurrCode++);
				continue;
			}
			break;
		}

		m_CurrLexeme += *(m_CurrCode++);
	}

	static STDString s_CommentStr;
	s_CommentStr = *m_CurrCode;
	if (m_CurrLexeme == HAZE_SINGLE_COMMENT)
	{
		while (s_CommentStr != H_TEXT("\n"))
		{
			m_CurrCode++;
			s_CommentStr = *m_CurrCode;

			if (STDString(m_CurrCode) == H_TEXT(""))
			{
				break;
			}
		}
		return GetNextToken();
	}
	else if (m_CurrLexeme == HAZE_MULTI_COMMENT_START)
	{
		HazeIsSpace(*m_CurrCode, &bNewLine);
		if (bNewLine)
		{
			IncLineCount();
		}

		s_CommentStr.resize(2);
		while (s_CommentStr != HAZE_MULTI_COMMENT_END)
		{
			m_CurrCode++;
			memcpy(s_CommentStr.data(), m_CurrCode, sizeof(x_HChar) * 2);

			HazeIsSpace(*m_CurrCode, &bNewLine);
			if (bNewLine)
			{
				IncLineCount();
			}
		}

		m_CurrCode += 2;
		return GetNextToken();
	}

	auto it = s_HashMap_Token.find(m_CurrLexeme);
	if (it != s_HashMap_Token.end())
	{
		m_CurrToken = it->second;
	}
	else if (IsNumberType(m_CurrLexeme, m_CurrToken)) { }
	else if (IsNumber(m_CurrLexeme))
	{
		m_CurrToken = HazeToken::Number;
	}
	else if (m_Compiler->IsClass(m_CurrLexeme) || (!m_CurrParseClass.empty() && m_CurrParseClass == m_CurrLexeme))
	{
		m_CurrToken = HazeToken::CustomClass;
	}
	else if (m_Compiler->IsEnum(m_CurrLexeme))
	{
		m_CurrToken = HazeToken::CustomEnum;
	}
	else
	{
		m_CurrToken = HazeToken::Identifier;
	}

	return m_CurrToken;
}

Unique<ASTBase> Parse::HandleParseExpression()
{
	return ParseExpression();
}

Unique<ASTBase> Parse::ParseExpression(int prec, HazeToken prevOpToken, Unique<ASTBase> left)
{
	if (m_IsParseError)
	{
		return nullptr;
	}

	Unique<ASTBase> right = ParseUnaryExpression();

	if (right)
	{
		return ParseBinaryOperateExpression2(prec, prevOpToken, Move(left), Move(right));
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseUnaryExpression()
{
	if (s_HashMap_OperatorPriority.find(m_CurrToken) == s_HashMap_OperatorPriority.end())
	{
		return ParsePrimary();
	}
	//else if (m_CurrToken == HazeToken::Mul)		//给指针指向的值赋值
	//{
	//	return ParsePrimary();
	//}
	//else if (m_CurrToken == HazeToken::BitAnd)	//取地址
	//{
	//	return ParseGetAddress();
	//}
	else if (m_CurrToken == HazeToken::Sub)		//取负数
	{
		return ParseNeg();
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseBinaryOperateExpression(int prec, HazeToken prevOpToken, Unique<ASTBase> prev, Unique<ASTBase> left)
{
	x_uint32 tempLineCount = m_LineCount;
	TempCurrCode temp(this);
	GetNextToken();

	auto curr = s_HashMap_OperatorPriority.find(m_CurrToken);
	if (curr == s_HashMap_OperatorPriority.end())
	{
		temp.Reset();

		if (prev)
		{
			return MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
				prevOpToken, prev, left);
		}
		else
		{
			return left;
		}
	}

	while (true)
	{
		HazeToken opToken = m_CurrToken;
		Unique<ASTBase> right = nullptr;
		if (m_CurrToken == HazeToken::ThreeOperatorStart)
		{
		}
		else if (TokenIs(HazeToken::Inc) || TokenIs(HazeToken::Dec))
		{
			
		}
		else
		{
			temp.Update();
			GetNextToken();
			if (prev && prec >= curr->second)
			{
				left = MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
					prevOpToken, prev, left);
			}

			temp.Update();
			STDString cacheLexeme = m_CurrLexeme;
			GetNextToken();
			auto next = s_HashMap_OperatorPriority.find(m_CurrToken);
			if (next != s_HashMap_OperatorPriority.end() && next->second < curr->second)
			{
				left = MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
					prevOpToken, prev, left);
			}
			temp.Reset();
			m_CurrLexeme = cacheLexeme;

			right = ParseExpression(curr->second, opToken, Move(left));
			if (!right)
			{
				return nullptr;
			}

			/*if (prev && prec < it->second)
			{
				left = MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
					prevOpToken, prev, right);
			}*/
		}

		HashMap<HazeToken, int>::iterator nextPrec;
		{
			temp.Update();
			GetNextToken();
			nextPrec = s_HashMap_OperatorPriority.find(m_CurrToken);
			if (nextPrec == s_HashMap_OperatorPriority.end())
			{
				temp.Reset();
				return MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
					opToken, left, right);
			}
		}

		if (curr->second < nextPrec->second)
		{
			//right = ParseBinaryOperateExpression(it->second + 1, Move(right));
			if (!right)
			{
				return nullptr;
			}

			left = MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
				opToken, left, right);
		}
		else if (nextPrec == s_HashMap_OperatorPriority.find(HazeToken::ThreeOperatorStart))
		{
			left = ParseThreeOperator(right ? MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), opToken, left, right)
				: Move(left));
		}
		else
		{
			temp.Reset();
			left = MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
				opToken, left, right);
		}
	}
}

Unique<ASTBase> Parse::ParseBinaryOperateExpression2(int prec, HazeToken prevOpToken, Unique<ASTBase> prev, Unique<ASTBase> left)
{
	x_uint32 tempLineCount = m_LineCount;
	TempCurrCode temp(this);
	GetNextToken();

	HazeToken currToken = m_CurrToken;
	auto curr = s_HashMap_OperatorPriority.find(m_CurrToken);
	if (curr == s_HashMap_OperatorPriority.end() || (prec > curr->second))
	{
		temp.Reset();
		if (prev)
		{
			return MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
				prevOpToken, prev, left);
		}
		else
		{
			return left;
		}
	}

	bool isRightAssociativity = curr->second == s_HashMap_OperatorPriority[HazeToken::Assign];
	Unique<ASTBase> right = nullptr;
	while (true)
	{
		//temp.Update();
		HazeToken opToken = m_CurrToken;
		temp.Update();
		GetNextToken();

		if (isRightAssociativity)
		{
			right = ParseExpression(curr->second, opToken, nullptr);
		}
		else
		{
			if (prec < curr->second)
			{
				if (prev)
				{
					left = ParseExpression(curr->second, opToken, Move(left));
				}
				else if (IsSingleOperator(opToken))
				{
					if (opToken == HazeToken::Inc)
					{
						left = MakeUnique<ASTInc>(m_Compiler, SourceLocation(tempLineCount), left, false);
					}
					else if (opToken == HazeToken::Dec)
					{
						left = MakeUnique<ASTDec>(m_Compiler, SourceLocation(tempLineCount), left, false);
					}
					temp.Reset();
				}
				else if (opToken == HazeToken::ThreeOperatorStart)
				{
					temp.Reset();
					left = ParseThreeOperator(Move(left));
				}
				else
				{
					left = ParseExpression(curr->second, opToken, Move(left));
				}
			}
			else if (prec == curr->second)
			{
				if (prev)
				{
					left = MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
						prevOpToken, prev, left);
				}

				left = ParseExpression(curr->second, opToken, Move(left));
			}
			else if (prec > curr->second)
			{
				temp.Reset();
				return left;
			}
		}

		temp.Update();
		GetNextToken();
		curr = s_HashMap_OperatorPriority.find(m_CurrToken);
		if (curr == s_HashMap_OperatorPriority.end() || prec > curr->second)
		{
			temp.Reset();
			break;
		}
	}

	if (left && right)
	{
		return MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
			currToken, left, right);
	}
	else if (left)
	{
		if (prev)
		{
			left = MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
				prevOpToken, prev, left);
		}
		
		return left;
	}

	return right;
}

Unique<ASTBase> Parse::ParsePrimary()
{
	HazeToken token = m_CurrToken;
	switch (token)
	{
		case HazeToken::Void:
		case HazeToken::Bool:
		case HazeToken::Int8:
		case HazeToken::UInt8:
		case HazeToken::Int16:
		case HazeToken::UInt16:
		case HazeToken::Int32:
		case HazeToken::UInt32:
		case HazeToken::Int64:
		case HazeToken::UInt64:
		case HazeToken::Float32:
		case HazeToken::Float64:
		case HazeToken::CustomClass:
		case HazeToken::CustomEnum:
		case HazeToken::DynamicClass:
		case HazeToken::String:
		case HazeToken::MultiVariable:
		case HazeToken::Reference:
		case HazeToken::ObjectBase:
		case HazeToken::Hash:
			return ParseVariableDefine();
		case HazeToken::Identifier:
			return ParseIdentifer();
		case HazeToken::This:
			return ParseIdentifer();
		case HazeToken::Number:
			return ParseNumberExpression();
		case HazeToken::StringMatch:
			return ParseStringText();
		case HazeToken::True:
		case HazeToken::False:
			return ParseBoolExpression();
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
		case HazeToken::Not:
			return ParseNot();
		case HazeToken::BitNeg:
			return ParseNeg();
		case HazeToken::NullPtr:
			return ParseNullPtr();
		case HazeToken::SizeOf:
			return ParseSizeOf();
		case HazeToken::GetAddress:
			return ParseGetAddress();
		case HazeToken::LeftBrace:
			return ParseLeftBrace();
		case HazeToken::RightBrace:
			break;
		default:
			PARSE_ERR_W("<%s>未能正确解析", m_CurrLexeme.c_str())
			break;
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseIdentifer(Unique<ASTBase> preAST, HazeToken preToken)
{
	Unique<ASTBase> ret = nullptr;
	STDString nameSpace;
	if (preToken == HazeToken::CustomClass)
	{
		nameSpace = m_CurrLexeme;
		if (!ExpectNextTokenIs(HazeToken::TwoColon, H_TEXT("命名空间空需要::")) ||
			!ExpectNextTokenIs(HazeToken::Identifier, H_TEXT("命名空间空需要自定义的命名")))
		{
			return nullptr;
		}
	}

	x_uint32 tempLineCount = m_LineCount;
	STDString identiferName;

	TempCurrCode temp(this);
	identiferName = m_CurrLexeme;

	bool moreExpect = false;
	if (ExpectNextTokenIs_NoParseError(HazeToken::LeftParentheses) && m_LineCount == tempLineCount)
	{
		//函数调用
		V_Array<Unique<ASTBase>> params;
		
		if (!ExpectNextTokenIs_NoParseError(HazeToken::RightParentheses))
		{
			while (true)
			{
				params.push_back(ParseExpression());

				if (ExpectNextTokenIs_NoParseError(HazeToken::Comma))
				{
					GetNextToken();
				}
				else
				{
					break;
				}
			}
		}

		if (TokenIs(HazeToken::RightParentheses, H_TEXT("函数调用需要 ) ")))
		{
			ret = MakeUnique<ASTFunctionCall>(m_Compiler, SourceLocation(tempLineCount),
				m_StackSectionSignal.top(), Move(identiferName), params, Move(preAST), nameSpace);

			moreExpect = true;
		}

	}
	else if (TokenIs(HazeToken::ClassAttr))
	{
		temp.Reset();
		moreExpect = true;
		ret = MakeUnique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
			Move(identiferName), nullptr, Move(preAST), nameSpace);
	}
	else if (TokenIs(HazeToken::Array) || preToken == HazeToken::Array)
	{
		m_IsParseArray = true;
		if (m_CurrToken == HazeToken::Array)
		{
			GetNextToken();
			auto indexExpression = ParseExpression();

			if (ExpectNextTokenIs(HazeToken::ArrayDefineEnd, H_TEXT("数组变量获得索引错误, 期望 ] ")))
			{
				m_IsParseArray = false;

				ret = MakeUnique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), 
					m_StackSectionSignal.top(), Move(identiferName), Move(indexExpression), Move(preAST), nameSpace);
				moreExpect = true;
			}
		}
		else
		{
			m_IsParseArray = false;
			auto indexExpression = ParseExpression();
			if (ExpectNextTokenIs(HazeToken::ArrayDefineEnd, H_TEXT("多数组变量获得索引错误, 期望 ] ")))
			{
				identiferName.clear();
				ret = MakeUnique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
					Move(identiferName), Move(indexExpression), Move(preAST), nameSpace);
				moreExpect = true;
			}
		}

		/*temp.Update();
		GetNextToken();*/
	}
	else if (TokenIs(HazeToken::TwoColon))
	{
		if (ExpectNextTokenIs_NoParseError(HazeToken::Identifier))
		{
			moreExpect = true;
			STDString name = m_CurrLexeme;
			ret = MakeUnique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(name), nullptr, Move(preAST), Move(identiferName));
		}
	}
	else
	{
		temp.Reset();
		ret = MakeUnique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), 
			m_StackSectionSignal.top(), Move(identiferName), nullptr, Move(preAST), nameSpace);
	}

	//这个里面需要考虑连续调用，如 甲->乙->丙 或者 甲->乙()->丙() 或着 甲()->乙()[0]->丙() 或着 甲()->乙->丙() 等
	if (moreExpect)
	{
		temp.Update();
		GetNextToken();
		if (TokenIs(HazeToken::ClassAttr) || TokenIs(HazeToken::Array) || TokenIs(HazeToken::CustomClass))
		{
			preToken = m_CurrToken;
			if (TokenIs(HazeToken::ClassAttr) && ExpectNextTokenIs_NoParseError(HazeToken::Identifier))
			{
				ret = ParseIdentifer(Move(ret), preToken);
			}
			else if (TokenIs(HazeToken::Array))
			{
				ret = ParseIdentifer(Move(ret), preToken);
			}
			else if (TokenIs(HazeToken::CustomClass))
			{
				if (temp.GetCacheToken() == HazeToken::ClassAttr)
				{
					ret = ParseIdentifer(Move(ret), HazeToken::CustomClass);
				}
				else
				{
					temp.Reset();
				}
			}
			else
			{
				PARSE_ERR_W("<%s>后变量名<%s>解析错误", identiferName.c_str(), m_CurrLexeme.c_str());
				return nullptr;
			}
		}
		else
		{
			temp.Reset();
		}
	}

	return ret;
}

Unique<ASTBase> Parse::ParseVariableDefine()
{
	x_uint32 tempLineCount = m_LineCount;

	HazeDefineVariable defineVar;
	defineVar.Type.BaseType = GetValueTypeByToken(m_CurrToken);
	GetValueType(defineVar.Type);

	TempCurrCode temp(this);
	GetNextToken();

	bool isTemplateVar = TokenIs(HazeToken::Less);
	x_uint32 templateTypeId = defineVar.Type.TypeId;
	if (isTemplateVar)
	{
		TemplateDefineTypes templateTypes;
		templateTypeId = ParseTemplateTypes(defineVar.Type, templateTypes);
		GetNextToken();
	}

	if (TokenIs(HazeToken::Array))
	{
		return ParseVariableDefine_Array(templateTypeId, Move(defineVar));
	}
	else if (IsStringType(defineVar.Type.BaseType))
	{
		return ParseVariableDefine_String(&temp, Move(defineVar));
	}
	else if (IsClassType(defineVar.Type.BaseType) && TokenIs(HazeToken::Identifier))
	{
		return ParseVariableDefine_Class(Move(defineVar));
	}
	else if (isTemplateVar)
	{
		if (IsObjectBaseType(defineVar.Type.BaseType))
		{
			return ParseVariableDefine_ObjectBase(templateTypeId, Move(defineVar));
		}
		else if (IsHashType(defineVar.Type.BaseType))
		{
			return ParseVariableDefine_Hash(templateTypeId, Move(defineVar));
		}
		else
		{
			return ParseVariableDefine_Function(templateTypeId, Move(defineVar));
		}
	}
	else if (IsMultiVariableType(defineVar.Type.BaseType))
	{
		if (TokenIs(HazeToken::RightParentheses, H_TEXT("多参数应是最后一个参数")))
		{
			temp.Reset();
			return ParseVariableDefine_MultiVariable();
		}
		else
		{
			return nullptr;
		}
	}
	else if (/*IsClassType(m_DefineVariable.Type.BaseType) && */TokenIs(HazeToken::TwoColon))
	{
		temp.Reset();
		m_CurrLexeme = *m_CompilerSymbol->GetSymbolByTypeId(defineVar.Type.TypeId); //*m_DefineVariable.Type.CustomName;
		return ParseIdentifer(nullptr, m_CurrToken);
	}

	if (TokenIs(HazeToken::Identifier))
	{
		defineVar.Name = m_CurrLexeme;

		temp.Update();
		GetNextToken();

		if (m_CurrToken == HazeToken::Assign)
		{
			GetNextToken();
   			Unique<ASTBase> expression = ParseExpression();

			return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount),m_StackSectionSignal.top(), Move(defineVar), Move(expression));
		}
		else
		{
			temp.Reset();
			return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), nullptr);
		}
	}
	else
	{
		PARSE_ERR_W("变量定义错误");
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseVariableDefine_MultiVariable()
{
	if (m_LibraryType == HazeLibraryType::Static || m_LibraryType == HazeLibraryType::DLL)
	{
		return MakeUnique<ASTVariableDefine_MultiVariable>(m_Compiler, m_LineCount);
	}
	else
	{
		PARSE_ERR_W("多参数只能定义在静态库或动态库函数中");
		return nullptr;
	}
}

Unique<ASTBase> Parse::ParseVariableDefine_Array(x_uint32 typeId, HazeDefineVariable&& defineVar)
{
	defineVar.Type.BaseType = HazeValueType::Array;
	defineVar.Type.TypeId = typeId;

	x_uint32 tempLineCount = m_LineCount;
	x_uint32 arrayDimension = 0;

	while (TokenIs(HazeToken::Array))
	{
		if (ExpectNextTokenIs(HazeToken::ArrayDefineEnd, H_TEXT("数组变量定义错误, 需要 ] ")))
		{
			arrayDimension++;
			if (ExpectNextTokenIs_NoParseError(HazeToken::Identifier))
			{
				break;
			}
		}
		else
		{
			return nullptr;
		}
	}

	ARRAY_TYPE_INFO(info, defineVar.Type.TypeId, arrayDimension);
	defineVar.Type.TypeId = m_CompilerSymbol->GetTypeInfoMap()->RegisterType(m_ModuleName, &info);

	if (TokenIs(HazeToken::Identifier, H_TEXT("数组对象变量名定义错误")))
	{
		defineVar.Name = m_CurrLexeme;

		TempCurrCode temp(this);
		if (ExpectNextTokenIs_NoParseError(HazeToken::Assign))
		{
			GetNextToken();
			Unique<ASTBase> expression = ParseExpression();

			return MakeUnique<ASTVariableDefine_Array>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), Move(expression));
		}
		else
		{
			temp.Reset();
			return MakeUnique<ASTVariableDefine_Array>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), nullptr);
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseVariableDefine_String(TempCurrCode* tempCode, HazeDefineVariable&& defineVar)
{
	x_uint32 tempLineCount = m_LineCount;

	if (TokenIs(HazeToken::Identifier))
	{
		defineVar.Name = m_CurrLexeme;

		TempCurrCode temp(this);

		GetNextToken();
		if (TokenIs(HazeToken::Assign))
		{
			GetNextToken();
			Unique<ASTBase> expression = ParseExpression();

			return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), Move(expression));
		}
		else if (m_IsParseClassData_Or_FunctionParam)
		{
			temp.Reset();
			return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), nullptr);
		}
		else
		{
			PARSE_ERR_W("字符对象定义需要括号\"(\"");
		}
	}
	else if (TokenIs(HazeToken::ClassAttr))
	{
		if (tempCode)
		{
			tempCode->Reset();
			m_CurrLexeme = TOKEN_STRING;
			Unique<ASTBase> expression = ParseIdentifer();

			return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), Move(expression));
		}
		else
		{
			PARSE_ERR_W("调用字符的函数错误");
		}
	}
	else
	{
		PARSE_ERR_W("字符变量或字符函数错误");
	}
	
	return nullptr;
}

Unique<ASTBase> Parse::ParseVariableDefine_Class(HazeDefineVariable&& defineVar)
{
	x_uint32 tempLineCount = m_LineCount;
	if (TokenIs(HazeToken::Identifier, H_TEXT("类对象变量定义错误")))
	{
		defineVar.Name = m_CurrLexeme;

		TempCurrCode temp(this);

		GetNextToken();
		if (TokenIs(HazeToken::LeftParentheses))
		{
			V_Array<Unique<ASTBase>> params;

			GetNextToken();
			while (!TokenIs(HazeToken::RightParentheses))
			{
				params.push_back(ParseExpression());
				if (!TokenIs(HazeToken::Comma))
				{
					break;
				}

				GetNextToken();
			}
			return MakeUnique<ASTVariableDefine_Class>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), nullptr, Move(params));
		}
		else if (TokenIs(HazeToken::Assign))
		{
			GetNextToken();
			Unique<ASTBase> expression = ParseExpression();

			return MakeUnique<ASTVariableDefine_Class>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), Move(expression));
		}
		else if (m_IsParseClassData_Or_FunctionParam)
		{
			temp.Reset();
			return MakeUnique<ASTVariableDefine_Class>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), nullptr);
		}
		else
		{
			PARSE_ERR_W("<%s>变量定义错误, 需要赋值", defineVar.Name.c_str());
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseVariableDefine_Function(x_uint32 templateTypeId, HazeDefineVariable&& defineVar)
{
	x_uint32 tempLineCount = m_LineCount;

	if (TokenIs(HazeToken::Identifier, H_TEXT("函数变量需要一个正确的名称")))
	{
		defineVar.Name = m_CurrLexeme;
		if (ExpectNextTokenIs_NoParseError(HazeToken::Assign))
		{
			if (NextTokenNotIs(HazeToken::Function))
			{
				Unique<ASTBase> expression = ParseExpression();
				return MakeUnique<ASTVariableDefine_Function>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), Move(expression), templateTypeId);
			}
			else
			{
				return ParseClosure(templateTypeId, Move(defineVar));
			}
		}
		else if (!m_IsParseClassData_Or_FunctionParam)
		{
			PARSE_ERR_W("函数变量<%s>定义错误, 需要赋予初始化值或空指针", defineVar.Name.c_str());
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseVariableDefine_ObjectBase(x_int32 templateTypeId, HazeDefineVariable&& defineVar)
{
	auto info = m_CompilerSymbol->GetTypeInfoMap()->GetTypeById(templateTypeId);
	if (!IsHazeBaseType(HAZE_ID_2_TYPE(info->_ObjectBase.TypeId1)))
	{
		PARSE_ERR_W("基本对象变量<%s>定义错误, 只能有一个类型且只能是基本类型", defineVar.Name.c_str());
	}
	else
	{
		x_uint32 tempLineCount = m_LineCount;
		if (TokenIs(HazeToken::Identifier, H_TEXT("基本类型对象需要一个正确的名称")))
		{
			defineVar.Name = m_CurrLexeme;
			defineVar.Type.BaseType = HazeValueType::ObjectBase;
			defineVar.Type.TypeId = templateTypeId;

			TempCurrCode temp(this);
			if (ExpectNextTokenIs_NoParseError(HazeToken::Assign))
			{
				GetNextToken();
				Unique<ASTBase> expression = ParseExpression();
				return MakeUnique<ASTVariableDefine_ObjectBase>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), Move(expression));
			}
			else if(!m_IsParseClassData_Or_FunctionParam)
			{
				PARSE_ERR_W("基本类型变量<%s>定义错误, 需要赋予初始化值或空指针", defineVar.Name.c_str());
			}

			temp.Reset();
			return MakeUnique<ASTVariableDefine_ObjectBase>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), nullptr);
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseVariableDefine_Hash(x_uint32 templateTypeId, HazeDefineVariable&& defineVar)
{
	x_uint32 tempLineCount = m_LineCount;

	if (TokenIs(HazeToken::Identifier, H_TEXT("哈希对象需要一个正确的名称")))
	{
		defineVar.Name = m_CurrLexeme;
		defineVar.Type.TypeId = templateTypeId;

		TempCurrCode temp(this);
		if (ExpectNextTokenIs_NoParseError(HazeToken::Assign))
		{
			GetNextToken();
			Unique<ASTBase> expression = ParseExpression();
			return MakeUnique<ASTVariableDefine_Hash>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), Move(expression), templateTypeId);
		}
		else if(!m_IsParseClassData_Or_FunctionParam)
		{
			PARSE_ERR_W("函数变量<%s>定义错误, 需要赋予初始化值或空指针", defineVar.Name.c_str());
		}

		temp.Reset();
		return MakeUnique<ASTVariableDefine_Hash>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), Move(defineVar), nullptr, templateTypeId);
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseClosure(x_uint32 templateTypeId, HazeDefineVariable&& defineVar)
{
	auto tempLineCount = m_LineCount;
	if (m_StackSectionSignal.top() == HazeSectionSignal::Local || m_StackSectionSignal.top() == HazeSectionSignal::Closure)
	{
		m_StackSectionSignal.push(HazeSectionSignal::Closure);
		if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("函数参数定义需要 (")))
		{
			V_Array<Unique<ASTBase>> params;

			HazeDefineVariable thisParam;
			thisParam.Name = HAZE_CLOSURE_NAME;
			thisParam.Type.BaseType = HazeValueType::Closure;
			params.push_back(MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), HazeSectionSignal::Closure, Move(thisParam), nullptr));


			GetNextToken();

			m_IsParseClassData_Or_FunctionParam = true;
			while (!TokenIs(HazeToken::LeftBrace) && !TokenIs(HazeToken::RightParentheses))
			{
				params.push_back(ParseExpression());
				if (!ExpectNextTokenIs_NoParseError(HazeToken::Comma))
				{
					break;
				}

				GetNextToken();
			}
			m_IsParseClassData_Or_FunctionParam = false;


			if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("函数体需要 {")))
			{
				x_uint32 startLineCount = m_LineCount;
				Unique<ASTBase> body = ParseMultiExpression();

				if (ExpectNextTokenIs(HazeToken::RightBrace, H_TEXT("函数体需要 }")))
				{
					m_StackSectionSignal.pop();
					//TemplateDefineTypes templateTypes;
					return MakeUnique<ASTVariableDefine_Closure>(m_Compiler, SourceLocation(tempLineCount), SourceLocation(startLineCount), SourceLocation(m_LineCount),
						m_StackSectionSignal.top(), Move(defineVar), body, templateTypeId, params);
				}
			}
		}
	}
	else
	{
		PARSE_ERR_W("匿名函数变量<%s>定义错误, 只能定义在函数或者匿名函数中", defineVar.Name.c_str());
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseStringText()
{
	x_uint32 tempLineCount = m_LineCount;
	GetNextToken();
	STDString text = m_CurrLexeme;

	return MakeUnique<ASTStringText>(m_Compiler, SourceLocation(tempLineCount), text);
}

Unique<ASTBase> Parse::ParseBoolExpression()
{
	x_uint32 tempLineCount = m_LineCount;
	HazeValue value;
	value.Value.Bool = m_CurrLexeme == TOKEN_TRUE;

	return MakeUnique<ASTBool>(m_Compiler, SourceLocation(tempLineCount), value);
}

Unique<ASTBase> Parse::ParseNumberExpression()
{
	x_uint32 tempLineCount = m_LineCount;
	HazeValue value;
	HazeValueType type = GetNumberDefaultType(m_CurrLexeme);

	if (m_IsParseArray)
	{
		type = HazeValueType::UInt64;
	}

	StringToHazeValueNumber(m_CurrLexeme, type, value);

	return MakeUnique<ASTNumber>(m_Compiler, SourceLocation(tempLineCount), type, value);
}

Unique<ASTBase> Parse::ParseIfExpression(/*bool recursion*/)
{
	x_uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("若 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto conditionExpression = ParseExpression();

		TempCurrCode temp(this);
		if (ExpectNextTokenIs(HazeToken::RightParentheses, H_TEXT("若 执行表达式期望捕捉 ) ")))
		{
			Unique<ASTBase> ifMultiExpression = nullptr;
			if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("若 执行表达式期望捕捉 { ")))
			{
				ifMultiExpression = ParseMultiExpression();

				if (ExpectNextTokenIs(HazeToken::RightBrace, H_TEXT("若 执行表达式期望捕捉 } ")))
				{
					temp.Update();
					GetNextToken();

					Unique<ASTBase> elseExpression = nullptr;

					if (TokenIs(HazeToken::Else))
					{
						GetNextToken();
						bool nextNotIf = m_CurrToken != HazeToken::If;
						//bool nextIfHasElseExpression = false;

						if (nextNotIf)
						{
							elseExpression = ParseMultiExpression();
							//nextIfHasElseExpression = true;

							if (!ExpectNextTokenIs(HazeToken::RightBrace, H_TEXT("否则 执行表达式期望捕捉 } ")))
							{
								return nullptr;
							}
						}
						else
						{
							elseExpression = ParseIfExpression(/*true*/);
							//nextIfHasElseExpression = dynamic_cast<ASTIfExpression*>(elseExpression.get())->HasElseExpression();

							if (!TokenIs(HazeToken::RightBrace, H_TEXT("否则 执行表达式期望捕捉 } ")))
							{
								return nullptr;
							}
						}

						//if (!recursion && nextIfHasElseExpression)
						//{
						//	GetNextToken();
						//}
					}
					else
					{
						temp.Reset();
					}

					return MakeUnique<ASTIfExpression>(m_Compiler, SourceLocation(tempLineCount), conditionExpression, ifMultiExpression, elseExpression);
				}
			}
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseForExpression()
{
	x_uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("循环表达式期望捕捉 (")))
	{
		GetNextToken();
		auto initExpression = ParseExpression();

		if (ExpectNextTokenIs(HazeToken::Comma, H_TEXT("循环表达式期望捕捉 , 来分隔语句")))
		{
			GetNextToken();
			auto conditionExpression = ParseExpression();

			if (ExpectNextTokenIs(HazeToken::Comma, H_TEXT("循环表达式期望捕捉 , 来分隔语句")))
			{
				GetNextToken();
				auto stepExpression = ParseExpression();
			
				if (ExpectNextTokenIs(HazeToken::RightParentheses, H_TEXT("循环表达式期望捕捉 ) ")) && 
					ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("循环 表达式块期望捕捉 {")))
				{
					auto multiExpression = ParseMultiExpression();

					if (ExpectNextTokenIs(HazeToken::RightBrace, H_TEXT("循环 表达式块期望捕捉 {")))
					{
						return MakeUnique<ASTForExpression>(m_Compiler, SourceLocation(tempLineCount), initExpression, 
							conditionExpression, stepExpression, multiExpression);
					}
				}
			}
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseWhileExpression()
{
	x_uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("当 表达式期望捕捉 ( ")))
	{
		GetNextToken();
		auto conditionExpression = ParseExpression();

		if (ExpectNextTokenIs(HazeToken::RightParentheses, H_TEXT("当 表达式期望捕捉 ) ")))
		{
			Unique<ASTBase> multiExpression = nullptr;
			if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("当 执行表达式期望捕捉 { ")))
			{
				multiExpression = ParseMultiExpression();

				if (ExpectNextTokenIs(HazeToken::RightBrace, H_TEXT("当 执行表达式期望捕捉 } ")))
				{
					return MakeUnique<ASTWhileExpression>(m_Compiler, SourceLocation(tempLineCount), conditionExpression, multiExpression);
				}

			}
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseBreakExpression()
{
	return MakeUnique<ASTBreakExpression>(m_Compiler, SourceLocation(m_LineCount));
}

Unique<ASTBase> Parse::ParseContinueExpression()
{
	return MakeUnique<ASTContinueExpression>(m_Compiler, SourceLocation(m_LineCount));
}

Unique<ASTBase> Parse::ParseReturn()
{
	x_uint32 tempLineCount = m_LineCount;

	TempCurrCode temp(this);
	GetNextToken();
	auto expression = ParseExpression();
	if (!expression)
	{
		temp.Reset();
	}

	return MakeUnique<ASTReturn>(m_Compiler, SourceLocation(tempLineCount), expression);
}

Unique<ASTBase> Parse::ParseNew()
{
	x_uint32 tempLineCount = m_LineCount;
	GetNextToken();

	HazeDefineVariable defineVar;
	defineVar.Type.BaseType = GetValueTypeByToken(m_CurrToken);
	GetValueType(defineVar.Type);

	V_Array<Unique<ASTBase>> arraySize;

	GetNextToken();
	bool isTemplateVar = TokenIs(HazeToken::Less);
	TemplateDefineTypes templateTypes;
	if (isTemplateVar)
	{
		ParseTemplateTypes(defineVar.Type, templateTypes);
		GetNextToken();
	}

	if (TokenIs(HazeToken::Array))
	{
		defineVar.Type.BaseType = HazeValueType::Array;
		m_IsParseArray = true;
		while (m_CurrToken == HazeToken::Array)
		{
			GetNextToken();
			if (TokenIs(HazeToken::ArrayDefineEnd))
			{
				HazeValue v;
				v.Value.UInt64 = 0;
				arraySize.push_back(MakeUnique<ASTNumber>(m_Compiler, SourceLocation(tempLineCount), HazeValueType::UInt64, v));
				break;
			}
			else
			{
				arraySize.push_back(ParseExpression());
				if (ExpectNextTokenIs_NoParseError(HazeToken::ArrayDefineEnd))
				{
					TempCurrCode temp(this);
					if (!ExpectNextTokenIs_NoParseError(HazeToken::Array))
					{
						temp.Reset();
						break;
					}
				}
			}

		}
		m_IsParseArray = false;
		return MakeUnique<ASTNew>(m_Compiler, SourceLocation(tempLineCount), Move(defineVar),
			Move(arraySize));
	}
	else
	{
		if (TokenIs(HazeToken::LeftParentheses, H_TEXT("生成表达式 期望 (")))
		{
			V_Array<Unique<ASTBase>> params;

			GetNextToken();
			while (!TokenIs(HazeToken::RightParentheses))
			{
				params.push_back(ParseExpression());
				if (!ExpectNextTokenIs_NoParseError(HazeToken::Comma))
				{
					break;
				}

				GetNextToken();
			}

			if (TokenIs(HazeToken::RightParentheses, H_TEXT("生成表达式 期望 ) ")))
			{
				return MakeUnique<ASTNew>(m_Compiler, SourceLocation(tempLineCount), Move(defineVar),
					Move(arraySize), Move(params));
			}
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseInc()
{
	x_uint32 tempLineCount = m_LineCount;

	GetNextToken();
	auto expression = ParseExpression();

	return MakeUnique<ASTInc>(m_Compiler, SourceLocation(tempLineCount), expression, true);
}

Unique<ASTBase> Parse::ParseDec()
{
	x_uint32 tempLineCount = m_LineCount;

	GetNextToken();
	auto expression = ParseExpression();

	return MakeUnique<ASTDec>(m_Compiler, SourceLocation(tempLineCount), expression, true);
}

Unique<ASTBase> Parse::ParseThreeOperator(Unique<ASTBase> Condition)
{
	x_uint32 tempLineCount = m_LineCount;
	auto iter = s_HashMap_OperatorPriority.find(HazeToken::ThreeOperatorStart);
	if (iter != s_HashMap_OperatorPriority.end())
	{
		auto conditionExpression = Move(Condition);

		if (TokenIs(HazeToken::ThreeOperatorStart, H_TEXT("三目表达式 需要 ? 符号")))
		{
			GetNextToken();
			auto leftExpression = ParseExpression(iter->second);

			if (ExpectNextTokenIs(HazeToken::Colon, H_TEXT("三目表达式 需要 : 符号")))
			{
				GetNextToken();
				auto rightExpression = ParseExpression(iter->second);

				return MakeUnique<ASTThreeExpression>(m_Compiler, SourceLocation(tempLineCount), conditionExpression, leftExpression, rightExpression);
			}
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseLeftParentheses()
{
	m_LeftParenthesesExpressionCount++;
	GetNextToken();

	Unique<ASTBase> expression = nullptr;
	if (IsCanCastToken(m_CurrToken))
	{
		HazeVariableType type;
		type.BaseType = GetValueTypeByToken(m_CurrToken);
		GetValueType(type);

		if (ExpectNextTokenIs_NoParseError(HazeToken::RightParentheses))
		{
			if (ExpectNextTokenIs_NoParseError(HazeToken::LeftParentheses))
			{
				expression = ParseExpression();
			}
			else
			{
				expression = ParseUnaryExpression();
			}
			return MakeUnique<ASTCast>(m_Compiler, m_LineCount, type, expression);
		}
	}
	else
	{
		expression = ParseExpression();
		GetNextToken();
		if (m_CurrToken == HazeToken::RightParentheses || m_LeftParenthesesExpressionCount == 0)
		{
			if (m_LeftParenthesesExpressionCount > 0)
			{
				m_LeftParenthesesExpressionCount--;
			}
			return expression;
		}
		else
		{
			PARSE_ERR_W("解析 ( 错误, 期望 ) ");
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseNeg()
{
	x_uint32 tempLineCount = m_LineCount;
	bool isNumberNeg = m_CurrToken == HazeToken::Sub;

	GetNextToken();
	auto expression = ParseExpression();

	return MakeUnique<ASTNeg>(m_Compiler, SourceLocation(tempLineCount), expression, isNumberNeg);
}

Unique<ASTBase> Parse::ParseNullPtr()
{
	x_uint32 tempLineCount = m_LineCount;
	return MakeUnique<ASTNullPtr>(m_Compiler, SourceLocation(tempLineCount));
}

Unique<ASTBase> Parse::ParseGetAddress()
{
	x_uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("取址需要 ( 符号")))
	{
		GetNextToken();
		auto expression = ParseExpression();
		if (ExpectNextTokenIs(HazeToken::RightParentheses, H_TEXT("取址需要 ) 符号")))
		{
			return MakeUnique<ASTGetAddress>(m_Compiler, SourceLocation(tempLineCount), expression);
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseNot()
{
	int tempLineCount = m_LineCount;

	GetNextToken();
	auto expression = ParseExpression(7000);

	return MakeUnique<ASTNot>(m_Compiler, SourceLocation(tempLineCount), expression);
}

Unique<ASTBase> Parse::ParseLeftBrace()
{
	//int a = 1 + 2 + 3 + 4 * 5 * (6 + 7 / 2 - 5) + 4;
	Unique<ASTBase> expression = nullptr;
	if (NextTokenNotIs(HazeToken::RightBrace))
	{
		expression = ParseMultiExpression();
		if (ExpectNextTokenIs(HazeToken::RightBrace, H_TEXT("区域期望 } ")))
		{
			return nullptr;
		}
	}
	else
	{
		expression = MakeUnique<ASTBase>(m_Compiler, SourceLocation(m_LineCount));
	}

	return expression;
}

Unique<ASTBase> Parse::ParseMultiExpression()
{
	V_Array<Unique<ASTBase>> expressions;

	TempCurrCode temp(this);
	if (NextTokenNotIs(HazeToken::RightBrace))
	{
		while (auto e = ParseExpression())
		{
			temp.Update();
			expressions.push_back(Move(e));

			if (ExpectNextTokenIs_NoParseError(HazeToken::RightBrace))
			{
				temp.Reset();
				break;
			}
		}
	}
	else
	{
		temp.Reset();
	}

	return MakeUnique<ASTMultiExpression>(m_Compiler, SourceLocation(m_LineCount), m_StackSectionSignal.top(), expressions);
}

Unique<ASTBase> Parse::ParseDataSection()
{
	if (ExpectNextTokenIs_NoParseError(HazeToken::LeftBrace))
	{
		V_Array<Unique<ASTBase>> expressions;

		EnableExpectIdentiferIsClassOrEnum();
		GetNextToken();
		while (auto e = ParseExpression())
		{
			expressions.push_back(Move(e));

			EnableExpectIdentiferIsClassOrEnum();
			if (ExpectNextTokenIs_NoParseError(HazeToken::RightBrace))
			{
				break;
			}
		}
		
		if (m_Compiler->IsStage1())
		{
			for (x_uint64 i = 0; i < expressions.size(); i++)
			{
				m_CompilerSymbol->Register_GlobalVariable(m_ModuleName, expressions[i]->GetDefine().Name, expressions[i]->GetDefine().Type, expressions[i]->GetLine());
			}
			return nullptr;
		}

		return MakeUnique<ASTDataSection>(m_Compiler, SourceLocation(m_LineCount), m_StackSectionSignal.top(), expressions);
	}

	return nullptr;
}

Unique<ASTFunctionSection> Parse::ParseFunctionSection()
{
	if (ExpectNextTokenIs_NoParseError(HazeToken::LeftBrace))
	{
		V_Array<Unique<ASTFunction>> functions;

		GetNextToken();
		while (m_CurrToken != HazeToken::RightBrace)
		{
			functions.push_back(ParseFunction());
			GetNextToken();
		}

		if (m_Compiler->IsStage1())
		{
			return nullptr;
		}

		return MakeUnique<ASTFunctionSection>(m_Compiler,/* SourceLocation(m_LineCount),*/ functions);
	}

	return nullptr;
}

Unique<ASTFunction> Parse::ParseFunction(const STDString* className, bool isClassPublic)
{
	m_StackSectionSignal.push(HazeSectionSignal::Local);
	x_uint32 tempLineCount = m_LineCount;
	HazeFunctionDesc desc = className ? HazeFunctionDesc::ClassNormal : HazeFunctionDesc::Normal;

	if (TokenIs(HazeToken::VirtualFunction))
	{
		desc = HazeFunctionDesc::ClassVirtual;
	}

	if (TokenIs(HazeToken::PureVirtualFunction))
	{
		desc = HazeFunctionDesc::ClassPureVirtual;
	}

	if (IS_VIRTUAL(desc) || IS_PURE_VIRTUAL(desc))
	{
		GetNextToken();
	}

	//获得函数返回类型及是自定义类型时获得类型名字
	HazeVariableType funcType;
	funcType.BaseType = GetValueTypeByToken(m_CurrToken);
	GetValueType(funcType);
	
	x_uint32 startLineCount = m_LineCount;
	STDString functionName;

	if (ExpectNextTokenIs_NoParseError(HazeToken::Identifier))
	{
		functionName = m_CurrLexeme;
		GetNextToken();
	}
	else if (className && funcType.TypeId > 0 && funcType.TypeId == m_CompilerSymbol->GetSymbolTypeId(*className) && !IS_VIRTUAL(desc) && !IS_PURE_VIRTUAL(desc))
	{
		//类构造函数
		funcType.BaseType = HazeValueType::Void;
		functionName = className->c_str();
	}
	else
	{
		PARSE_ERR_W("未能找到有效的函数命名");
		return nullptr;
	}

	if (TokenIs(HazeToken::LeftParentheses, H_TEXT("函数参数解析错误, 需要 ( ")))
	{
		V_Array<Unique<ASTBase>> params;
		if (className && !className->empty())
		{
			HazeDefineVariable thisParam;
			thisParam.Name = TOKEN_THIS;
			thisParam.Type.BaseType = HazeValueType::Class;
			thisParam.Type.TypeId = m_CompilerSymbol->GetSymbolTypeId(*className);

			params.push_back(MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), HazeSectionSignal::Local, Move(thisParam), nullptr));
		}

		EnableExpectIdentiferIsClassOrEnum();
		GetNextToken();

		m_IsParseClassData_Or_FunctionParam = true;
		bool startCheckDefaultParam = false;
		while (!TokenIs(HazeToken::LeftBrace) && !TokenIs(HazeToken::RightParentheses))
		{
			params.push_back(ParseExpression());

			auto ast = dynamic_cast<ASTVariableDefine*>(params.back().get());
			if (!startCheckDefaultParam)
			{
				startCheckDefaultParam = ast->HasAssignExpression();
			}
			else if (!ast->HasAssignExpression())
			{
				PARSE_ERR_W("默认参数之后的参数也必须有默认值");
			}

			if (!ExpectNextTokenIs_NoParseError(HazeToken::Comma))
			{
				break;
			}

			EnableExpectIdentiferIsClassOrEnum();
			GetNextToken();
		}
		m_IsParseClassData_Or_FunctionParam = false;

		if (m_Compiler->IsStage1())
		{
			ParseStage1Spin(this);

			RegisterFunctionSymbol(functionName, funcType.TypeId, params, desc, className, isClassPublic);
			return nullptr;
		}

		if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("函数体需要 {")))
		{
			startLineCount = m_LineCount;
			Unique<ASTBase> body = ParseMultiExpression();

			if (ExpectNextTokenIs(HazeToken::RightBrace, H_TEXT("函数体需要 }")))
			{
				m_StackSectionSignal.pop();
				return MakeUnique<ASTFunction>(m_Compiler, SourceLocation(startLineCount), SourceLocation(m_LineCount),
					m_StackSectionSignal.top(), functionName, funcType, params, Move(body), desc);
			}
		}
	}

	return nullptr;
}

Unique<ASTLibrary> Parse::ParseLibrary()
{
	HazeLibraryType libType = GetHazeLibraryTypeByToken(m_CurrToken);
	m_LibraryType = libType;

	GetNextToken();
	STDString standardLibraryName = m_CurrLexeme;

	m_StackSectionSignal.push(HazeSectionSignal::Global);

	if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("库需要 {")))
	{
		V_Array<Unique<ASTClassDefine>> classDefines;
		V_Array<Unique<ASTFunctionDefine>> functionDefines;

		GetNextToken();
		while (m_CurrToken == HazeToken::Function || m_CurrToken == HazeToken::Class)
		{
			if (m_CurrToken == HazeToken::Function)
			{
				auto functions = ParseLibrary_FunctionDefine();
				for (auto& iter : functions)
				{
					functionDefines.push_back(Move(iter));
				}
			}
		}

		if (ExpectNextTokenIs(HazeToken::RightBrace, H_TEXT("库需要 }")))
		{
			m_StackSectionSignal.pop();
			m_LibraryType = HazeLibraryType::Normal;
			return MakeUnique<ASTLibrary>(m_Compiler, /*SourceLocation(m_LineCount),*/ standardLibraryName,
				libType, functionDefines, classDefines);
		}
	}

	return nullptr;
}

V_Array<Unique<ASTFunctionDefine>> Parse::ParseLibrary_FunctionDefine()
{
	V_Array<Unique<ASTFunctionDefine>> functionDefines;

	if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("库函数群需要 {")))
	{
		while (!ExpectNextTokenIs_NoParseError(HazeToken::RightBrace))
		{
			m_StackSectionSignal.push(HazeSectionSignal::Local);

			//获得函数返回类型及是自定义类型时获得类型名字
			HazeVariableType funcType;
			funcType.BaseType = GetValueTypeByToken(m_CurrToken);
			GetValueType(funcType);

			//获得函数名
			if (ExpectNextTokenIs(HazeToken::Identifier, H_TEXT("库函数命名错误")))
			{
				STDString functionName = m_CurrLexeme;
				if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("函数参数定义需要 (")))
				{
					V_Array<Unique<ASTBase>> params;

					GetNextToken();

					m_IsParseClassData_Or_FunctionParam = true;
					while (!TokenIs(HazeToken::LeftBrace) && !TokenIs(HazeToken::RightParentheses))
					{
						params.push_back(ParseExpression());
						if (!ExpectNextTokenIs_NoParseError(HazeToken::Comma))
						{
							break;
						}

						GetNextToken();
					}
					m_IsParseClassData_Or_FunctionParam = false;

					m_StackSectionSignal.pop();

					RegisterFunctionSymbol(functionName, funcType.TypeId, params, HazeFunctionDesc::Normal, nullptr, false);
					if (TokenIs(HazeToken::RightParentheses, H_TEXT("函数最后需要 ) ")))
					{
						functionDefines.push_back(MakeUnique<ASTFunctionDefine>(m_Compiler, /*SourceLocation(m_LineCount),*/ Move(functionName), funcType, params));
					}
				}
			}
		}

		TokenIs(HazeToken::RightBrace, H_TEXT("库函数群需要 }"));
	}

	return functionDefines;
}

Unique<ASTBase> Parse::ParseImportModule()
{
	if (ExpectNextTokenIs_NoParseError(HazeToken::Identifier) || m_Compiler->GetModule(m_CurrLexeme))
	{
		/*if (m_Compiler->IsStage1())
		{
			m_Compiler->ParseModuleByImportPath(m_CurrLexeme);
			return nullptr;
		}*/

		STDString name = m_CurrLexeme;
		return MakeUnique<ASTImportModule>(m_Compiler, SourceLocation(m_LineCount), Move(name));
	}
	else
	{
		PARSE_ERR_W("引入模块<%s>错误, 模块名不是有效文本", m_CurrLexeme.c_str());
	}

	return nullptr;
}

Unique<ASTClass> Parse::ParseClass()
{
	if (ExpectNextTokenIs_NoParseError(HazeToken::Identifier) || TokenIs(HazeToken::CustomClass))
	{
		m_CurrParseClass = m_CurrLexeme;
		m_CompilerSymbol->RegisterSymbol(m_CurrParseClass);

		STDString name = m_CurrLexeme;
		V_Array<STDString> parentClasses;

		GetNextToken();
		if (TokenIs(HazeToken::Colon))
		{
			if (ExpectNextTokenIs_NoParseError(HazeToken::CustomClass))
			{
				while (TokenIs(HazeToken::CustomClass))
				{
					parentClasses.push_back(m_CurrLexeme);

					GetNextToken();
					if (TokenIs(HazeToken::Comma))
					{
						GetNextToken();
					}
					else if (TokenIs(HazeToken::LeftBrace))
					{
						break;
					}
					else
					{
						PARSE_ERR_W("类<%s>继承<%s>错误", name.c_str(), m_CurrLexeme.c_str());
					}
				}
			}
			else if (m_Compiler->IsStage1())
			{
				while (TokenIs(HazeToken::CustomClass) || TokenIs(HazeToken::Identifier))
				{
					parentClasses.push_back(m_CurrLexeme);
					m_CompilerSymbol->AddModuleRefSymbol(m_ModuleName, m_CurrLexeme);

					GetNextToken();
					if (TokenIs(HazeToken::Comma))
					{
						GetNextToken();
					}
					else if (TokenIs(HazeToken::LeftBrace))
					{
						break;
					}
					else
					{
						PARSE_ERR_W("类<%s>继承<%s>错误", name.c_str(), m_CurrLexeme.c_str());
					}
				}
			}
			else
			{
				PARSE_ERR_W("类<%s>继承<%s>错误", name.c_str(), m_CurrLexeme.c_str());
			}
		}

		if (TokenIs(HazeToken::LeftBrace))
		{
			m_StackSectionSignal.push(HazeSectionSignal::Class);

			V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> classDatas;
			Unique<ASTClassFunctionSection> classFunctions;

			GetNextToken();
			while (m_CurrToken == HazeToken::Data || m_CurrToken == HazeToken::Function)
			{
				if (m_CurrToken == HazeToken::Data)
				{
					if (classDatas.size() == 0)
					{
						classDatas = ParseClassData();
						GetNextToken();
					}
					else
					{
						PARSE_ERR_W("类中相同的区域<%s>只能存在一种", name.c_str());
					}
				}
				else if (m_CurrToken == HazeToken::Function)
				{
					classFunctions = ParseClassFunction(name);
					GetNextToken();
				}
			}

			m_StackSectionSignal.pop();

			if (m_Compiler->IsStage1())
			{
				RegisterClassData(name, parentClasses, classDatas);
				return nullptr;
			}

			return MakeUnique<ASTClass>(m_Compiler, /*SourceLocation(m_LineCount),*/ name, parentClasses, classDatas, classFunctions);
		}
	}
	else
	{
		PARSE_ERR_W("类名<%s>错误", m_CurrLexeme.c_str());
	}

	return nullptr;
}

V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> Parse::ParseClassData()
{
	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> classDatas;

	auto HasData = [](decltype(classDatas)& data, HazeDataDesc scopeType) -> bool
	{
		for (size_t i = 0; i < data.size(); i++)
		{
			if (data[i].first == scopeType)
			{
				return true;
			}
		}

		return false;
	};

	if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("类数据需要 {")))
	{

		GetNextToken();
		m_IsParseClassData_Or_FunctionParam = true;
		while (m_CurrToken == HazeToken::ClassPublic || m_CurrToken == HazeToken::ClassPrivate)
		{
			if (m_CurrToken == HazeToken::ClassPublic)
			{
				if (!HasData(classDatas, HazeDataDesc::ClassMember_Local_Public))
				{
					classDatas.push_back(Move(
						Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Public,
						Move(V_Array<Unique<ASTBase>>{}) })));
					if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("类数据成员区域需要 { ")))
					{
						EnableExpectIdentiferIsClassOrEnum();
						while (NextTokenNotIs(HazeToken::RightBrace))
						{
							classDatas.back().second.push_back(ParseExpression());
							EnableExpectIdentiferIsClassOrEnum();
						}
					}
				}
				else
				{
					PARSE_ERR_W("类的公有区域只能定义一次");
				}
			}
			else if (m_CurrToken == HazeToken::ClassPrivate)
			{
				if (!HasData(classDatas, HazeDataDesc::ClassMember_Local_Private))
				{
					classDatas.push_back(Move(
						Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Private,
						Move(V_Array<Unique<ASTBase>>{}) })));
					if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("类数据成员区域需要 { ")))
					{
						EnableExpectIdentiferIsClassOrEnum();
						while (NextTokenNotIs(HazeToken::RightBrace))
						{
							classDatas.back().second.push_back(ParseExpression());
							EnableExpectIdentiferIsClassOrEnum();
						}
					}
				}
				else
				{
					PARSE_ERR_W("类的私有区域只能定义一次");
				}
			}

			GetNextToken();
		}
		m_IsParseClassData_Or_FunctionParam = false;

		if (TokenIs(HazeToken::RightBrace, H_TEXT("解析错误: 类数据区域需要 }")))
		{
			return classDatas;
		}
	}

	return classDatas;
}

Unique<ASTClassFunctionSection> Parse::ParseClassFunction(const STDString& className)
{
	if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("类函数定义需要 {")))
	{
		GetNextToken();

		V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>> classFunctions;
		//	={ {HazeDataDesc::ClassFunction_Local_Public, {}} };

		auto HasData = [](decltype(classFunctions)& functions, HazeDataDesc scopeType) -> bool
		{
			for (size_t i = 0; i < functions.size(); i++)
			{
				if (functions[i].first == scopeType)
				{
					return true;
				}
			}

			return false;
		};

		TempCurrCode temp(this);
		while (m_CurrToken == HazeToken::ClassPublic || m_CurrToken == HazeToken::ClassPrivate)
		{
			if (m_CurrToken == HazeToken::ClassPublic)
			{
				if (!(HasData(classFunctions, HazeDataDesc::ClassFunction_Local_Public)))
				{
					classFunctions.push_back(Move(
						Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Public,
						Move(V_Array<Unique<ASTFunction>>{}) })));
					if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("类函数成员区域需要 { ")))
					{
						GetNextToken();
						while (m_CurrToken != HazeToken::RightBrace)
						{
							classFunctions.back().second.push_back(ParseFunction(&className, true));
							GetNextToken();
						}

						if (TokenIs(HazeToken::RightBrace, H_TEXT("类函数成员区域需要 } ")))
						{
							temp.Update();
							if (NextTokenNotIs(HazeToken::ClassPrivate))
							{
								temp.Reset();
							}
						}
					}
				}
			}
			else if (m_CurrToken == HazeToken::ClassPrivate)
			{
				if (!(HasData(classFunctions, HazeDataDesc::ClassFunction_Local_Private)))
				{
					classFunctions.push_back(Move(
						Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Private,
						Move(V_Array<Unique<ASTFunction>>{}) })));
					if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("类函数成员区域需要 { ")))
					{
						GetNextToken();
						while (m_CurrToken != HazeToken::RightBrace)
						{
							classFunctions.back().second.push_back(ParseFunction(&className));
							GetNextToken();
						}

						if (TokenIs(HazeToken::RightBrace, H_TEXT("类函数成员区域需要 } ")))
						{
							temp.Update();
							if (NextTokenNotIs(HazeToken::ClassPublic))
							{
								temp.Reset();
							}
						}
					}
				}
			}
		}

		if (!ExpectNextTokenIs_NoParseError(HazeToken::RightBrace))
		{
			PARSE_ERR_W("类<%s>函数需要 }, 或者需要<%s><%s>关键字", className.c_str(), TOKEN_CLASS_DATA_PUBLIC, TOKEN_CLASS_DATA_PRIVATE);
		}

		if (m_Compiler->IsStage1())
		{
			return nullptr;
		}

		return MakeUnique<ASTClassFunctionSection>(m_Compiler, /*SourceLocation(m_LineCount),*/ classFunctions);
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseSizeOf()
{
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("获得字节大小需要 （")))
	{
		GetNextToken();

		HazeVariableType type;
		Unique<ASTBase> variable = nullptr;
		if (TokenIs(HazeToken::Identifier))
		{
			variable = ParseExpression();
		}
		else
		{
			type.BaseType = GetValueTypeByToken(m_CurrToken);
			GetValueType(type);
		}

		if (ExpectNextTokenIs(HazeToken::RightParentheses, H_TEXT("获得字节大小需要 )")))
		{
			return MakeUnique<ASTSizeOf>(m_Compiler, m_LineCount, type, variable);
		}
	}

	return nullptr;
}

Unique<ASTEnum> Parse::ParseEnum()
{
	m_StackSectionSignal.push(HazeSectionSignal::Enum);
	if (ExpectNextTokenIs_NoParseError(HazeToken::Identifier))
	{
		m_CompilerSymbol->RegisterSymbol(m_CurrLexeme);

		int line = m_LineCount;
		STDString name = m_CurrLexeme;

		if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("枚举定义缺少 { 符号")))
		{
			V_Array<Pair<STDString, Unique<ASTBase>>> enums;

			TempCurrCode temp(this);
			while (ExpectNextTokenIs_NoParseError(HazeToken::Identifier))
			{
				enums.push_back({ m_CurrLexeme, nullptr });

				temp.Update();
				if (ExpectNextTokenIs_NoParseError(HazeToken::Assign))
				{
					GetNextToken();
					enums.back().second = ParseExpression();
				}
				else if (TokenIs(HazeToken::RightBrace))
				{
					break;
				}
				else
				{
					temp.Reset();
				}
			}

			if (TokenIs(HazeToken::RightBrace, H_TEXT("枚举定义缺少 } 符号")))
			{
				m_StackSectionSignal.pop();

				if (m_Compiler->IsStage1())
				{
					RegisterEnumData(name, enums);
					return nullptr;
				}

				// 枚举不再需要类型
				return MakeUnique<ASTEnum>(m_Compiler, line, name, enums);
			}
		}
	}
	else if (TokenIs(HazeToken::CustomEnum))
	{
		ParseStage1Spin(this);
	}
	else
	{
		PARSE_ERR_W("%s, 获得<%s>", H_TEXT("枚举名错误"), m_CurrLexeme.c_str());
	}

	m_StackSectionSignal.pop();
	return nullptr;
}

//Unique<ASTTemplateBase> Parse::ParseTemplateClass(V_Array<HString>& templateTypes)
//{
//	m_CurrParseClass = m_CurrLexeme;
//	HString name = m_CurrLexeme;
//	V_Array<HString> parentClasses;
//
//	GetNextToken();
//	if (TokenIs(HazeToken::Colon))
//	{
//		//暂时不支持继承模板类
//		if (ExpectNextTokenIs_NoParseError(HazeToken::CustomClass))
//		{
//			while (TokenIs(HazeToken::CustomClass))
//			{
//				parentClasses.push_back(m_CurrLexeme);
//
//				GetNextToken();
//				if (TokenIs(HazeToken::Comma))
//				{
//					GetNextToken();
//				}
//				else if (TokenIs(HazeToken::LeftBrace))
//				{
//					break;
//				}
//				else
//				{
//					PARSE_ERR_W("类<%s>继承<%s>错误", name.c_str(), m_CurrLexeme.c_str());
//				}
//			}
//		}
//		else
//		{
//			PARSE_ERR_W("类<%s>继承<%s>错误", name.c_str(), m_CurrLexeme.c_str());
//		}
//	}
//
//	if (TokenIs(HazeToken::LeftBrace))
//	{
//		m_StackSectionSignal.push(HazeSectionSignal::Class);
//
//		V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> classDatas;
//		Unique<ASTClassFunctionSection> classFunctions;
//
//		GetNextToken();
//		while (m_CurrToken == HazeToken::Data || m_CurrToken == HazeToken::Function)
//		{
//			if (m_CurrToken == HazeToken::Data)
//			{
//				if (classDatas.size() == 0)
//				{
//					classDatas = ParseClassData();
//				}
//				else
//				{
//					PARSE_ERR_W("类中相同的区域<%s>只能存在一种", name.c_str());
//				}
//			}
//			else if (m_CurrToken == HazeToken::Function)
//			{
//				classFunctions = ParseClassFunction(name);
//			}
//		}
//
//		m_StackSectionSignal.pop();
//		return nullptr; //return MakeUnique<ASTTemplateClass>(m_Compiler, /*SourceLocation(m_LineCount),*/ name, parentClasses, classDatas, classFunctions);
//	}
//
//	return nullptr;
//}
//Unique<ASTTemplateBase> Parse::ParseTemplateFunction(V_Array<HString>& templateTypes)
//{
//	return Unique<ASTTemplateBase>();
//}

bool Parse::ExpectNextTokenIs(HazeToken token, const x_HChar* errorInfo, bool parseError)
{
	HazeToken NextToken = GetNextToken();
	if (token != NextToken)
	{
		if (parseError)
		{
			m_IsParseError = true;
		}

		if (errorInfo)
		{
			PARSE_ERR_W("%s, 获得<%s>", errorInfo, m_CurrLexeme.c_str());
		}
		return false;
	}

	return true;
}

bool Parse::TokenIs(HazeToken token, const x_HChar* errorInfo)
{
	if (token != m_CurrToken)
	{
		if (errorInfo)
		{
			PARSE_ERR_W("%s", errorInfo);
		}
		return false;
	}

	return true;
}

bool Parse::IsHazeSignalToken(const x_HChar* hChar, const x_HChar*& outChar, x_uint32 charSize)
{
	static HashSet<STDString> s_HashSet_TokenText =
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
		TOKEN_CLASS_ATTR,
		TOKEN_INC, TOKEN_DEC, TOKEN_ADD_ASSIGN, TOKEN_SUB_ASSIGN, TOKEN_MUL_ASSIGN, TOKEN_DIV_ASSIGN, TOKEN_MOD_ASSIGN, TOKEN_SHL_ASSIGN, TOKEN_SHR_ASSIGN,
		TOKEN_BIT_AND_ASSIGN, TOKEN_BIT_OR_ASSIGN, TOKEN_BIT_XOR_ASSIGN,
		TOKEN_QUESTIOB_COLON, TOKEN_TWO_COLON,
	};

	static STDString s_WS;

	s_WS.resize(charSize);
	memcpy(s_WS.data(), hChar, sizeof(x_HChar) * charSize);

	auto iter = s_HashSet_TokenText.find(s_WS);
	if (iter != s_HashSet_TokenText.end())
	{
		if (m_IsParseTemplate && (*iter == TOKEN_LEFT_MOVE || *iter == TOKEN_RIGHT_MOVE))
		{
			return false;
		}

		outChar = iter->c_str();
	}

	return iter != s_HashSet_TokenText.end();
}

bool Parse::IsNumberType(const STDString& str, HazeToken& outToken)
{
	auto iter = s_NumberMap.find(str);
	if (iter != s_NumberMap.end())
	{
		outToken = iter->second;
		return true;
	}

	return false;
}

void Parse::GetValueType(HazeVariableType& inType)
{
	switch (m_CurrToken)
	{
	case HazeToken::MultiVariable:
		break;
	case HazeToken::CustomClass:
	{
		inType.TypeId = m_CompilerSymbol->GetSymbolTypeId(m_CurrLexeme);
		if (inType.TypeId == 0)
		{
			PARSE_ERR_W("获得类<%s>的类型错误", m_CurrLexeme.c_str());
		}
	}
		break;
	case HazeToken::CustomEnum:
	{
		inType.TypeId = m_CompilerSymbol->GetSymbolTypeId(m_CurrLexeme);
		if (inType.TypeId == 0)
		{
			PARSE_ERR_W("获得枚举<%s>的类型错误", m_CurrLexeme.c_str());
		}
	}
		break;
	case HazeToken::Reference:
	{
		auto s = m_CurrLexeme.substr(0, m_CurrLexeme.length() - 1);
		auto iter = s_HashMap_Token.find(s);
		auto type = GetValueTypeByToken(iter->second);
		if (IsHazeBaseType(type))
		{
			inType.TypeId = HAZE_TYPE_ID(type);
		}
		else
		{
			PARSE_ERR_W("获得<%s>引用类型错误, 必须是基本类型", s.c_str());
		}
	}
		break;
	case HazeToken::String:
	{
		inType = HAZE_VAR_TYPE(HazeValueType::String);
		//inType.BaseType = HazeValueType::String;
	}
		break;
	case HazeToken::Void:
	case HazeToken::Bool:
	case HazeToken::Int8:
	case HazeToken::UInt8:
	case HazeToken::Int16:
	case HazeToken::UInt16:
	case HazeToken::Int32:
	case HazeToken::UInt32:
	case HazeToken::Int64:
	case HazeToken::UInt64:
	case HazeToken::Float32:
	case HazeToken::Float64:
	case HazeToken::Function:
	case HazeToken::DynamicClass:
		inType.TypeId = HAZE_TYPE_ID(GetValueTypeByToken(m_CurrToken));
		break;
	case HazeToken::ObjectBase:
	case HazeToken::Hash:
		return;
	case HazeToken::Identifier:
		if (m_Compiler->IsStage1())
		{
			return;
		}
	default:
		PARSE_ERR_W("获得<%s>变量类型错误", m_CurrLexeme.c_str());
		break;
	}
}

x_uint32 Parse::ParseTemplateTypes(HazeVariableType baseType, TemplateDefineTypes& templateTypes)
{
	struct Scope
	{
		Scope(bool* scopeValue) : ScopeValue(scopeValue) { *ScopeValue = true; }
		~Scope() { *ScopeValue = false; }
	private:
		bool* ScopeValue;
	};

	Scope scope(&m_IsParseTemplate);
	V_Array<x_uint32> typeIds;
	x_uint32 arrayDimension = 0;

	//if (IsUseTemplateType(baseType.BaseType))
	{
		while (true)
		{
			GetNextToken();
			HazeVariableType type;
			type.BaseType = GetValueTypeByToken(m_CurrToken);
			GetValueType(type);
			TempCurrCode temp(this);

			if (ExpectNextTokenIs_NoParseError(HazeToken::Less) && IsUseTemplateType(type.BaseType))
			{
				//TemplateDefineTypes types;
				typeIds.push_back(ParseTemplateTypes(type, templateTypes));
				m_IsParseTemplate = true;

				/*TemplateDefineType t(true, MakeShare<TemplateDefineTypes>(Move(types)), nullptr);
				templateTypes.Types.push_back(t);*/
			}
			else
			{
				temp.Reset();
				typeIds.push_back(type.TypeId);
				/*TemplateDefineType t(false, nullptr, MakeShare<HazeNewDefineType>(type));
				templateTypes.Types.push_back(t);*/
			}

			temp.Update();
			while (ExpectNextTokenIs_NoParseError(HazeToken::Array))
			{
				if (CanArray(type.BaseType) && ExpectNextTokenIs_NoParseError(HazeToken::ArrayDefineEnd))
				{
					arrayDimension++;
					temp.Update();
				}
				else
				{
					PARSE_ERR_W("解析数组类型错误");
					return 0;
				}
			}

			temp.Reset();
			if (arrayDimension > 0)
			{
				/*templateTypes.Types.back().Type->BaseType.TypeId = m_Compiler->GetTypeInfoMap()->RegisterType(CAST_COMPLEX_INFO(typeInfo));
				templateTypes.Types.back().Type->BaseType.BaseType = HazeValueType::Array;*/

				ARRAY_TYPE_INFO(info, typeIds.back(), arrayDimension);
				typeIds.back() = m_CompilerSymbol->GetTypeInfoMap()->RegisterType(m_ModuleName, & info);
			}

			if (ExpectNextTokenIs_NoParseError(HazeToken::Greater))
			{
				break;
			}
		}
	}
	/*else
	{
		PARSE_ERR_W("解析模板类型错误");
	}*/

	HazeComplexTypeInfo info;

#undef TYPE_INFO_VAR
#define TYPE_INFO_VAR(INFO)
	switch (baseType.BaseType)
	{
		case HazeValueType::Hash:
			HASH_TYPE_INFO(info, typeIds[0], typeIds[1]);
			break;
		//case HazeValueType::Array:
		//	ARRAY_TYPE_INFO(info, typeIds[0], arrayDimension);
		//	break;
		case HazeValueType::ObjectBase:
			OBJ_BASE_TYPE_INFO(info, typeIds[0]);
			break;
		default:
			//其他类型都是函数
			return m_CompilerSymbol->GetTypeInfoMap()->RegisterType(m_ModuleName, baseType.TypeId, Move(typeIds));
	}
#undef TYPE_INFO_VAR

	return  m_CompilerSymbol->GetTypeInfoMap()->RegisterType(m_ModuleName, &info);
}

void Parse::RegisterClassData(const STDString& name, V_Array<STDString>& parents, V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>>& classDatas)
{
	bool isPublicFirst = classDatas.size() > 0 && classDatas[0].first == HazeDataDesc::ClassMember_Local_Public;

	V_Array<Pair<STDString, x_uint32>> publicMembers;
	V_Array<Pair<STDString, x_uint32>> privateMembers;
	for (x_uint64 i = 0; i < classDatas.size(); i++)
	{
		if (classDatas[i].first == HazeDataDesc::ClassMember_Local_Public)
		{
			for (x_uint64 j = 0; j < classDatas[i].second.size(); j++)
			{
				publicMembers.push_back({ Move(const_cast<HazeDefineVariable&>(classDatas[i].second[j]->GetDefine()).Name), classDatas[i].second[j]->GetDefine().Type.TypeId });
			}
		}
		else
		{
			for (x_uint64 j = 0; j < classDatas[i].second.size(); j++)
			{
				privateMembers.push_back({ Move(const_cast<HazeDefineVariable&>(classDatas[i].second[j]->GetDefine()).Name), classDatas[i].second[j]->GetDefine().Type.TypeId });
			}
		}
	}

	m_CompilerSymbol->Register_Class(m_ModuleName, name, parents, Move(publicMembers), Move(privateMembers), isPublicFirst);
}

void Parse::RegisterEnumData(const STDString& name, V_Array<Pair<STDString, Unique<ASTBase>>>& members)
{
	V_Array<Pair<STDString, int>> newMembers(members.size());

	for (x_uint64 i = 0; i < members.size(); i++)
	{
		newMembers[i].first = Move(members[i].first);
		
		if (members[i].second)
		{
			newMembers[i].second = members[i].second->GetValue().Value.Int32;
		}
		else
		{
			newMembers[i].second = i == 0 ? 0 : newMembers[i - 1].second + 1;
		}
	}

	m_CompilerSymbol->Register_Enum(m_ModuleName, name, Move(newMembers));
}

void Parse::RegisterFunctionSymbol(const STDString& functionName, x_uint32 funcType, V_Array<Unique<ASTBase>>& params, HazeFunctionDesc desc, const STDString* className, bool isClassPublic)
{
	V_Array<HazeDefineVariableView> funcParams(params.size());
	for (x_uint64 i = 0; i < funcParams.size(); i++)
	{
		funcParams[i] = params[i]->GetDefine();
	}

	m_CompilerSymbol->Register_Function(m_ModuleName, functionName, funcType, Move(funcParams), desc, className, isClassPublic);
}

void Parse::EnableExpectIdentiferIsClassOrEnum()
{
	if (m_Compiler->IsStage1())
	{
		m_IsExpectIdentiferIsClass_Or_Enum = true;
	}
}

void Parse::OnHitExpectIdentiferIsClassOrEnum()
{
	m_IsExpectIdentiferIsClass_Or_Enum = false;
}

//void Parse::ParseVariableType()
//{
//	int pointerLevel = 1;
//	if (TokenIs(HazeToken::CustomClass))
//	{
//		m_DefineVariable.Type.CustomName = m_CurrLexeme;
//	}
//	else if (TokenIs(HazeToken::PointerBase) || TokenIs(HazeToken::ReferenceBase))
//	{
//		if (m_IsParseTemplate)
//		{
//			GetTemplateRealValueType(m_CurrLexeme, m_DefineVariable.Type);
//		}
//		else
//		{
//			m_DefineVariable.Type.SecondaryType = GetPointerBaseType(m_CurrLexeme);
//		}
//		m_DefineVariable.Type.CustomName = H_TEXT("");
//	}
//	else if (m_CurrToken == HazeToken::PointerClass || m_CurrToken == HazeToken::ReferenceClass)
//	{
//		m_DefineVariable.Type.SecondaryType = HazeValueType::Class;
//		if (m_IsParseTemplate)
//		{
//			GetTemplateRealValueType(m_CurrLexeme, m_DefineVariable.Type);
//		}
//		else
//		{
//			m_DefineVariable.Type.CustomName = GetPointerClassType(m_CurrLexeme);
//		}
//	}
//	else if (m_CurrToken == HazeToken::PointerPointer)
//	{
//		if (m_IsParseTemplate)
//		{
//			GetTemplateRealValueType(m_CurrLexeme, m_DefineVariable.Type);
//		}
//		else
//		{
//			m_DefineVariable.Type.SecondaryType = GetPointerBaseType(m_CurrLexeme);
//		}
//	}
//	else if (TokenIs(HazeToken::MultiVariable))
//	{
//		m_DefineVariable.Name = HAZE_MULTI_PARAM_NAME;
//	}
//
//	if (TokenIs(HazeToken::PointerBase) || TokenIs(HazeToken::PointerClass))
//	{
//		while (GetNextToken() == HazeToken::Mul)
//		{
//			pointerLevel += 1;
//		}
//
//		if (pointerLevel > 1)
//		{
//			HAZE_TO_DO(Parse pointer pointer !);
//		}
//	}
//	else
//	{
//		GetNextToken();
//	}
//}

void Parse::IncLineCount()
{
	m_LineCount++;

#if 0
	x_HChar code[40];
	memcpy(code, m_CurrCode + 1, sizeof(code));
	code[39] = '\0';

	HAZE_LOG_ERR_W("Line %d %s\n", m_LineCount, code);
#endif


	//#if HAZE_DEBUG_ENABLE
	//
	//	if (Insert)
	//	{
	//		Compiler->InsertLineCount(m_LineCount);
	//	}
	//
	//#endif // HAZE_DEBUG_ENABLE
}