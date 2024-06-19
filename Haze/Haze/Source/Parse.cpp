﻿#include <cctype>
#include <fstream>

#include "HazeHeader.h"
#include "HazeVM.h"
#include "Parse.h"
#include "ASTBase.h"
#include "ASTFunction.h"
#include "ASTClass.h"
#include "ASTLibrary.h"
#include "ASTTemplateClass.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"

#include "HazeLogDefine.h"
#include <cstdarg>

#include "HazeTokenText.h"

static HAZE_STRING MUL_STR = TOKEN_MUL;

//因为会出现嵌套解析同一种AST的情况，所以需要记录不同的行

static std::unordered_map<HAZE_STRING, HazeToken> s_HashMap_Token =
{
	{ TOKEN_VOID, HazeToken::Void },
	{ TOKEN_BOOL, HazeToken::Bool },
	{ TOKEN_BYTE, HazeToken::Byte },
	{ TOKEN_CHAR, HazeToken::Char },
	{ TOKEN_INT, HazeToken::Int },
	{ TOKEN_FLOAT, HazeToken::Float },
	{ TOKEN_LONG, HazeToken::Long },
	{ TOKEN_DOUBLE, HazeToken::Double },

	{ TOKEN_UNSIGNED_INT, HazeToken::UnsignedInt },
	{ TOKEN_UNSIGNED_LONG, HazeToken::UnsignedLong },

	{ TOKEN_ARRAY_START, HazeToken::Array },
	{ TOKEN_ARRAY_END, HazeToken::ArrayDefineEnd },
	{ TOkEN_ARRAY_LENGTH, HazeToken::ArrayLength },

	{ TOKEN_STRING_MATCH, HazeToken::StringMatch },

	{ TOKEN_FUNCTION, HazeToken::Function },

	{ TOKEN_ENUM, HazeToken::Enum },

	{ TOKEN_CLASS, HazeToken::Class },
	{ TOKEN_CLASS_DATA, HazeToken::m_ClassDatas },
	{ TOKEN_CLASS_DATA_PUBLIC, HazeToken::ClassPublic },
	{ TOKEN_CLASS_DATA_PRIVATE, HazeToken::ClassPrivate },
	{ TOKEN_CLASS_DATA_PROTECTED, HazeToken::ClassProtected },

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
	{ TOKEN_FOR_STEP, HazeToken::ForStep },

	{ TOKEN_BREAK, HazeToken::Break },
	{ TOKEN_CONTINUE, HazeToken::Continue },
	{ TOKEN_RETURN, HazeToken::Return },

	{ TOKEN_WHILE, HazeToken::While },

	{ TOKEN_CAST, HazeToken::Cast },

	{ TOKEN_VIRTUAL, HazeToken::VirtualFunction },
	{ TOKEN_PUREVIRTUAL, HazeToken::PureVirtualFunction },

	{ TOKEN_DEFINE, HazeToken::Define },

	{ TOKEN_STANDARD_LIBRARY, HazeToken::StandardLibrary },
	{ TOKEN_DLL_LIBRARY, HazeToken::DLLLibrary },
	{ TOKEN_IMPORT_MODULE, HazeToken::ImportModule },

	{ TOKEN_MULTI_VARIABLE, HazeToken::MultiVariable },

	{ TOKEN_NEW, HazeToken::New },

	{ TOKEN_QUESTION_MARK, HazeToken::ThreeOperatorStart },
	{ TOKEN_QUESTIOB_COLON, HazeToken::Colon },

	{ TOKEN_NULL_PTR, HazeToken::NullPtr },

	{ TOKEN_TYPENAME, HazeToken::TypeName },
	{ TOKEN_TEMPLATE, HazeToken::Template },

	{ TOKEN_SIZE_OF, HazeToken::SizeOf },
};

const std::unordered_map<HAZE_STRING, HazeToken>& GetHashMap_Token()
{
	return s_HashMap_Token;
}

static std::unordered_map<HazeToken, int> s_HashMap_OperatorPriority =
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

	//{ HazeToken::Not, 7000 },
	{ HazeToken::Inc, 9000 },
	{ HazeToken::Dec, 9000 },

	//{ HazeToken::LeftParentheses, 10000 },
};

static HazeValueType GetPointerBaseType(const HAZE_STRING& str)
{
	HazeToken token = s_HashMap_Token.find(str.substr(0, str.length() - 1))->second;
	return GetValueTypeByToken(token);
}

static HAZE_STRING GetPointerClassType(const HAZE_STRING& str)
{
	return str.substr(0, str.length() - 1);
}

//static void GetParseType(HazeDefineType& Type, const HAZE_STRING& Str)
//{
//	if (Type.PrimaryType == HazeValueType::PointerBase)
//	{
//		Type.SecondaryType = GetPointerBaseType(Str);
//	}
//	else if (Type.PrimaryType == HazeValueType::PointerClass)
//	{
//		Type.CustomName = GetPointerClassType(Str);
//	}
//	else if (Type.PrimaryType == HazeValueType::Class)
//	{
//		Type.CustomName = Str;
//	}
//}

Parse::Parse(HazeCompiler* compiler)
	: m_Compiler(compiler), m_CurrCode(nullptr), m_CurrToken(HazeToken::None),
	m_LeftParenthesesExpressionCount(0), m_LineCount(1), m_NeedParseNextStatement(false), 
	m_IsParseTemplate(false), m_TemplateTypes(nullptr), m_TemplateRealTypes(nullptr)
{
}

Parse::~Parse()
{
}

void Parse::InitializeFile(const HAZE_STRING& filePath)
{
	HAZE_BINARY_IFSTREAM fs(filePath);
	fs.imbue(std::locale("chs"));

	std::string content(std::istreambuf_iterator<char>(fs), {});
	content = UTF8_2_GB2312(content.c_str());
	m_CodeText = String2WString(content);

	m_CurrCode = m_CodeText.c_str();
	fs.close();
}

void Parse::InitializeString(const HAZE_STRING& str, uint32 startLine)
{
	m_CodeText = str;
	m_CurrCode = m_CodeText.c_str();
	m_LineCount = startLine;
}

void Parse::ParseContent()
{
	m_StackSectionSignal.push(HazeSectionSignal::Global);
	GetNextToken();

	while (!TokenIsNone(m_CurrToken))
	{
		switch (m_CurrToken)
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
			//case HazeToken::String:
		case HazeToken::Identifier:
		case HazeToken::CustomClass:
		case HazeToken::PointerBase:
		case HazeToken::PointerClass:
		case HazeToken::ReferenceBase:
		case HazeToken::ReferenceClass:
		case HazeToken::PointerFunction:
		case HazeToken::PointerPointer:
		{
			auto ast = ParseExpression();
			ast->CodeGen();
		}
		break;
		case HazeToken::Class:
		{
			auto ast = ParseClass();
			ast->CodeGen();
		}
		break;
		case HazeToken::Function:
		{
			auto ast = ParseFunctionSection();
			ast->CodeGen();
		}
		break;
		case HazeToken::Enum:
		{
			auto ast = ParseEnum();
			ast->CodeGen();
		}
		break;
		case HazeToken::Template:
		{
			ParseTemplate();
		}
		break;
		case HazeToken::StandardLibrary:
		case HazeToken::DLLLibrary:
		{
			auto ast = ParseLibrary();
			ast->CodeGen();
		}
		break;
		case HazeToken::ImportModule:
		{
			auto ast = ParseImportModule();
			ast->CodeGen();
			GetNextToken();
		}
		break;
		default:
			PARSE_ERR_W("未能找到生成相应Token的AST处理");
			break;
		}
	}

	m_StackSectionSignal.pop();
}

void Parse::ParseTemplateContent(const HAZE_STRING& moduleName, const HAZE_STRING& templateName, const std::vector<HAZE_STRING>& templateTypes, const std::vector<HazeDefineType>& templateRealTypes)
{
	m_Compiler->MarkParseTemplate(true, &moduleName);
	m_IsParseTemplate = true;
	m_TemplateTypes = &templateTypes;
	m_TemplateRealTypes = &templateRealTypes;

	m_CurrParseClass = templateName;
	std::vector<HAZE_STRING> parentClasses;
	if (ExpectNextTokenIs(HazeToken::Colon))
	{
		//暂时不支持继承模板类
		if (ExpectNextTokenIs(HazeToken::CustomClass))
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
					HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n"), templateName.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n"), templateName.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
		}
	}

	if (TokenIs(HazeToken::LeftBrace))
	{
		m_StackSectionSignal.push(HazeSectionSignal::Class);

		std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> classDatas;
		std::unique_ptr<ASTClassFunctionSection> classFunctions;

		GetNextToken();
		while (m_CurrToken == HazeToken::m_ClassDatas || m_CurrToken == HazeToken::Function)
		{
			if (m_CurrToken == HazeToken::m_ClassDatas)
			{
				if (classDatas.size() == 0)
				{
					classDatas = ParseClassData();
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类中相同的区域<%s>只能存在一种! <%s>文件<%d>行!"), templateName.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
			else if (m_CurrToken == HazeToken::Function)
			{
				classFunctions = ParseClassFunction(templateName);
			}
		}

		m_StackSectionSignal.pop();

		GetNextToken();

		m_CurrParseClass.clear();

		HAZE_STRING className = templateName;
		std::make_unique<ASTClass>(m_Compiler, /*SourceLocation(m_LineCount),*/ className,
			parentClasses, classDatas, classFunctions)->CodeGen();
	}

	m_IsParseTemplate = false;
	m_TemplateTypes = nullptr;
	m_TemplateRealTypes = nullptr;
	m_Compiler->MarkParseTemplate(false);
}

HazeToken Parse::GetNextToken()
{
	bool bNewLine = false;
	while (HazeIsSpace(*m_CurrCode, &bNewLine))
	{
		if (bNewLine)
		{
			IncLineCount();
		}
		m_CurrCode++;
	}

	if (HAZE_STRING(m_CurrCode) == HAZE_TEXT(""))
	{
		IncLineCount();
		m_CurrToken = HazeToken::None;
		m_CurrLexeme = HAZE_TEXT("");
		return HazeToken::None;
	}

	//Match Token
	m_CurrLexeme.clear();
	const HAZE_CHAR* signal;
	while (!HazeIsSpace(*m_CurrCode, &bNewLine) || m_CurrToken == HazeToken::StringMatch)
	{
		if (bNewLine)
		{
			IncLineCount(true);
		}

		if (IsHazeSignalToken(m_CurrCode, signal))
		{
			if (m_CurrToken == HazeToken::StringMatch)
			{
				static HAZE_STRING tempString;
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
			else if (HAZE_STRING(signal) == TOKEN_ARRAY_START)
			{
				if (m_CurrLexeme.empty())
				{
					m_CurrLexeme += *m_CurrCode++;
					m_CurrToken = HazeToken::Array;
					return m_CurrToken;
				}
			}
			else if (m_CurrToken == HazeToken::Array && HAZE_STRING(signal) == TOKEN_ARRAY_END)
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
			else if (m_CurrLexeme.length() == 0 || IsPointerOrRef(m_CurrLexeme + *m_CurrCode, m_CurrToken)
				|| IsHazeSignalToken(m_CurrCode - 1, signal, 2))
			{
				m_CurrLexeme += *(m_CurrCode++);
			}
			break;
		}

		m_CurrLexeme += *(m_CurrCode++);
	}

	static HAZE_STRING s_CommentStr;
	s_CommentStr = *m_CurrCode;
	if (m_CurrLexeme == HAZE_SINGLE_COMMENT)
	{
		while (s_CommentStr != HAZE_TEXT("\n"))
		{
			m_CurrCode++;
			s_CommentStr = *m_CurrCode;
		}
		return GetNextToken();
	}
	else if (m_CurrLexeme == HAZE_MULTI_COMMENT_START)
	{
		s_CommentStr.resize(2);
		while (s_CommentStr != HAZE_MULTI_COMMENT_END)
		{
			m_CurrCode++;
			memcpy(s_CommentStr.data(), m_CurrCode, sizeof(HAZE_CHAR) * 2);

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
	else if (IsNumber(m_CurrLexeme))
	{
		m_CurrToken = HazeToken::Number;
	}
	else if (IsPointerOrRef(m_CurrLexeme, m_CurrToken))
	{
	}
	else if (m_Compiler->IsClass(m_CurrLexeme) || m_CurrParseClass == m_CurrLexeme || m_Compiler->IsTemplateClass(m_CurrLexeme))
	{
		m_CurrToken = HazeToken::CustomClass;
	}
	else
	{
		m_CurrToken = HazeToken::Identifier;
	}

	return m_CurrToken;
}

std::unique_ptr<ASTBase> Parse::HandleParseExpression()
{
	return ParseExpression();
}

std::unique_ptr<ASTBase> Parse::ParseExpression(int prec)
{
	if (m_NeedParseNextStatement)
	{
		m_NeedParseNextStatement = false;
	}

	std::unique_ptr<ASTBase> left = ParseUnaryExpression();

	if (left)
	{
		return ParseBinaryOperateExpression(prec, std::move(left));
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseUnaryExpression()
{
	if (s_HashMap_OperatorPriority.find(m_CurrToken) == s_HashMap_OperatorPriority.end())
	{
		return ParsePrimary();
	}
	else if (m_CurrToken == HazeToken::Mul)		//给指针指向的值赋值
	{
		return ParsePrimary();
	}
	else if (m_CurrToken == HazeToken::BitAnd)	//取地址
	{
		return ParseGetAddress();
	}
	else if (m_CurrToken == HazeToken::Sub)		//取负数
	{
		return ParseNeg();
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseBinaryOperateExpression(int prec, std::unique_ptr<ASTBase> left)
{
	uint32 tempLineCount = m_LineCount;
	while (true)
	{
		if (m_NeedParseNextStatement)
		{
			return left;
		}

		auto it = s_HashMap_OperatorPriority.find(m_CurrToken);
		if (it == s_HashMap_OperatorPriority.end())
		{
			return left;
		}

		if (it->second < prec)
		{
			return left;
		}

		HazeToken opToken = m_CurrToken;
		std::unique_ptr<ASTBase> right = nullptr;
		if (m_CurrToken == HazeToken::ThreeOperatorStart)
		{
		}
		else if (TokenIs(HazeToken::Inc) || TokenIs(HazeToken::Dec))
		{
			GetNextToken();
		}
		else
		{
			GetNextToken();
			right = ParseExpression(it->second);
			if (!right)
			{
				return nullptr;
			}
		}

		auto nextPrec = s_HashMap_OperatorPriority.find(m_CurrToken);
		if (nextPrec == s_HashMap_OperatorPriority.end())
		{
			return std::make_unique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), opToken, left, right);
		}

		if (it->second < nextPrec->second)
		{
			right = ParseBinaryOperateExpression(it->second + 1, std::move(right));
			if (!right)
			{
				return nullptr;
			}

			left = std::make_unique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), opToken, left, right);
		}
		else if (nextPrec == s_HashMap_OperatorPriority.find(HazeToken::ThreeOperatorStart))
		{
			left = ParseThreeOperator(right ? std::make_unique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), opToken, left, right) : std::move(left));
		}
		else
		{
			left = std::make_unique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), opToken, left, right);
		}
	}
}

std::unique_ptr<ASTBase> Parse::ParsePrimary()
{
	HazeToken token = m_CurrToken;
	switch (token)
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
		//case HazeToken::String:
	case HazeToken::CustomClass:
	case HazeToken::PointerBase:
	case HazeToken::PointerClass:
	case HazeToken::ReferenceBase:
	case HazeToken::ReferenceClass:
	case HazeToken::PointerFunction:
	case HazeToken::PointerPointer:
		return ParseVariableDefine();
	case HazeToken::Identifier:
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
	case HazeToken::LeftBrace:
		return ParseLeftBrace();
	case HazeToken::Not:
		return ParseNot();
	case HazeToken::Mul:	//pointer value
		return ParsePointerValue();
	case HazeToken::BitAnd:
		return ParseGetAddress();
	case HazeToken::BitNeg:
		return ParseNeg();
	case HazeToken::NullPtr:
		return ParseNullPtr();
	case HazeToken::ArrayLength:
		return ParseArrayLength();
	case HazeToken::SizeOf:
		return ParseSizeOf();
	default:
		break;
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseIdentifer()
{
	uint32 tempLineCount = m_LineCount;
	std::unique_ptr<ASTBase> ret = nullptr;
	HAZE_STRING identiferName = m_CurrLexeme;
	std::vector<std::unique_ptr<ASTBase>> indexExpression;

	if (GetNextToken() == HazeToken::LeftParentheses && m_LineCount == tempLineCount)
	{
		//函数调用
		std::vector<std::unique_ptr<ASTBase>> params;
		while (true)
		{
			if (GetNextToken() == HazeToken::RightParentheses)
			{
				break;
			}

			params.push_back(ParseExpression());

			if (!TokenIs(HazeToken::Comma))
			{
				break;
			}
		}

		GetNextToken();
		return std::make_unique<ASTFunctionCall>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), identiferName, params);
	}
	else if (m_CurrToken == HazeToken::Array)
	{
		auto cacheToken = m_CurrToken;
		if (ExpectNextTokenIs(HazeToken::ArrayDefineEnd))
		{
			m_CurrCode -= (m_CurrLexeme.length() * 2 + identiferName.length());
			GetNextToken();
			return ParseVariableDefine();
		}
		else
		{
			m_CurrToken = cacheToken;
			m_CurrCode -= m_CurrLexeme.length();
			while (m_CurrToken == HazeToken::Array)
			{
				GetNextToken();
				indexExpression.push_back(ParseExpression());

				GetNextToken();
			}
		}
		

		ret = std::make_unique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), identiferName, indexExpression);
	}
	else
	{
		ret = std::make_unique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), identiferName, indexExpression);
	}

	return ret;
}

std::unique_ptr<ASTBase> Parse::ParseVariableDefine()
{
	uint32 tempLineCount = m_LineCount;
	m_DefineVariable.Name.clear();
	m_DefineVariable.Type.Reset();

	m_DefineVariable.Type.PrimaryType = GetValueTypeByToken(m_CurrToken);

	int pointerLevel = 1;
	GetValueType(m_DefineVariable.Type);
	GetNextToken();

	std::vector<std::unique_ptr<ASTBase>> arraySize;
	if (TokenIs(HazeToken::Array))
	{
		ResetArrayVariableType(m_DefineVariable.Type);

		while (m_CurrToken == HazeToken::Array)
		{
			if (ExpectNextTokenIs(HazeToken::ArrayDefineEnd))
			{
				GetNextToken();
				break;
			}
			else
			{
				arraySize.push_back(ParseExpression());
				GetNextToken();
			}
		}
	}

	bool isTemplateVar = TokenIs(HazeToken::Less) && m_DefineVariable.Type.PrimaryType == HazeValueType::Class;
	if (TokenIs(HazeToken::Identifier) || isTemplateVar)
	{
		m_DefineVariable.Name = m_CurrLexeme;

		GetNextToken();

		if (m_CurrToken == HazeToken::Assign)
		{
			GetNextToken();//吃掉赋值符号
			std::unique_ptr<ASTBase> expression = ParseExpression();

			return std::make_unique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount),m_StackSectionSignal.top(),
				m_DefineVariable, std::move(expression), std::move(arraySize), pointerLevel);
		}
		else if ((m_CurrToken == HazeToken::RightParentheses || m_CurrToken == HazeToken::Comma) && m_StackSectionSignal.top() == HazeSectionSignal::Local)
		{
			//函数调用
			return std::make_unique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
				m_DefineVariable, nullptr);
		}
		else if (m_DefineVariable.Type.PrimaryType == HazeValueType::Class)
		{
			if (m_CurrToken == HazeToken::LeftParentheses)
			{
				//类对象定义
				GetNextToken();
				if (m_CurrToken == HazeToken::RightParentheses)
				{
					GetNextToken();
					return std::make_unique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), m_DefineVariable, nullptr, std::move(arraySize), pointerLevel);
				}
				else
				{
					HAZE_LOG_ERR_W("解析错误, 暂时不支持类的构造函数有参数!");
				}
			}
			else if (isTemplateVar)
			{
				std::vector<HazeDefineType> templateTypes;
				while (true)
				{
					HazeDefineType type;
					type.PrimaryType = GetValueTypeByToken(m_CurrToken);
					GetValueType(type);

					templateTypes.push_back(type);

					if (ExpectNextTokenIs(HazeToken::Greater))
					{
						break;
					}
					else if (m_CurrToken == HazeToken::Comma)
					{
						GetNextToken();
					}
				}

				if (TokenIs(HazeToken::Greater))
				{
					if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("模板类对象命名错误")))
					{
						m_DefineVariable.Name = m_CurrLexeme;
						if (ExpectNextTokenIs(HazeToken::LeftParentheses) && ExpectNextTokenIs(HazeToken::RightParentheses))
						{
							GetNextToken();
							return  std::make_unique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount),
								m_StackSectionSignal.top(), m_DefineVariable, nullptr, std::move(arraySize), pointerLevel, &templateTypes);
						}
					}
				}
			}
			else
			{
				HAZE_LOG_ERR(HAZE_TEXT("解析错误 类对象定义需要括号\"(\" <%s>文件<%d>行!!\n"), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
			}
		}
		else
		{
			if (IsPointerType(m_DefineVariable.Type.PrimaryType))
			{
				PARSE_ERR_W("指针类型需要初始化");
			}

			return std::make_unique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), m_DefineVariable, nullptr, std::move(arraySize), pointerLevel);
		}
	}
	else if (m_CurrToken == HazeToken::LeftParentheses)
	{
		//函数指针或数组指针
		if (ExpectNextTokenIs(HazeToken::Mul) && ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("函数指针或者数组指针需要一个正确的名称")))
		{
			std::vector<HazeDefineType> paramTypes;
			paramTypes.push_back(m_DefineVariable.Type);

			m_DefineVariable.Name = m_CurrLexeme;

			if (ExpectNextTokenIs(HazeToken::RightParentheses))
			{
				if (ExpectNextTokenIs(HazeToken::LeftParentheses))
				{
					m_DefineVariable.Type.PrimaryType = HazeValueType::PointerFunction;
					GetNextToken();
					while (true)
					{
						HazeDefineType type;
						type.PrimaryType = GetValueTypeByToken(m_CurrToken);
						GetValueType(type);

						paramTypes.push_back(type);

						if (ExpectNextTokenIs(HazeToken::RightParentheses))
						{
							break;
						}
						else if (m_CurrToken == HazeToken::Comma)
						{
							GetNextToken();
						}
					}

					if (ExpectNextTokenIs(HazeToken::Assign))
					{
						GetNextToken();

						std::unique_ptr<ASTBase> expression = ParseExpression();

						return std::make_unique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), m_DefineVariable, std::move(expression), std::move(arraySize), pointerLevel);
					}
				}
				else
				{
					PARSE_ERR_W("指针变量<%s>定义错误", m_DefineVariable.Name.c_str());
				}
			}
		}
	}
	else if (m_DefineVariable.Name.empty())
	{
		PARSE_ERR_W("变量定义错误");
	}

	return std::make_unique<ASTVariableDefine>(m_Compiler, SourceLocation(m_LineCount), m_StackSectionSignal.top(), m_DefineVariable, nullptr, std::move(arraySize), pointerLevel);;
}

std::unique_ptr<ASTBase> Parse::ParseStringText()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	HAZE_STRING text = m_CurrLexeme;

	GetNextToken();
	return std::make_unique<ASTStringText>(m_Compiler, SourceLocation(tempLineCount), text);
}

std::unique_ptr<ASTBase> Parse::ParseBoolExpression()
{
	uint32 tempLineCount = m_LineCount;
	HazeValue value;
	value.Value.Bool = m_CurrLexeme == TOKEN_TRUE;

	GetNextToken();
	return std::make_unique<ASTBool>(m_Compiler, SourceLocation(tempLineCount), value);
}

std::unique_ptr<ASTBase> Parse::ParseNumberExpression()
{
	uint32 tempLineCount = m_LineCount;
	HazeValue value;
	HazeValueType type = GetNumberDefaultType(m_CurrLexeme);

	StringToHazeValueNumber(m_CurrLexeme, type, value);

	GetNextToken();
	return std::make_unique<ASTNumber>(m_Compiler, SourceLocation(tempLineCount), type, value);
}

std::unique_ptr<ASTBase> Parse::ParseIfExpression(bool recursion)
{
	uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("若 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto conditionExpression = ParseExpression();

		std::unique_ptr<ASTBase> ifMultiExpression = nullptr;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("若 执行表达式期望捕捉 { ")))
		{
			ifMultiExpression = ParseMultiExpression();
			GetNextToken();
		}

		std::unique_ptr<ASTBase> elseExpression = nullptr;

		if (m_CurrToken == HazeToken::Else)
		{
			GetNextToken();
			bool nextNotIf = m_CurrToken != HazeToken::If;
			bool nextIfHasElseExpression = false;

			if (nextNotIf)
			{
				elseExpression = ParseMultiExpression();
				nextIfHasElseExpression = true;
			}
			else
			{
				elseExpression = ParseIfExpression(true);
				nextIfHasElseExpression = dynamic_cast<ASTIfExpression*>(elseExpression.get())->HasElseExpression();
			}

			if (!recursion && nextIfHasElseExpression)
			{
				GetNextToken();
			}
		}

		return std::make_unique<ASTIfExpression>(m_Compiler, SourceLocation(tempLineCount), conditionExpression, ifMultiExpression, elseExpression);
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseForExpression()
{
	uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("循环 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto initExpression = ParseExpression();

		GetNextToken();
		auto conditionExpression = ParseExpression();

		GetNextToken();
		auto stepExpression = ParseExpression();

		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("循环 表达式块期望捕捉 {")))
		{
			auto multiExpression = ParseMultiExpression();

			GetNextToken();
			return std::make_unique<ASTForExpression>(m_Compiler, SourceLocation(tempLineCount), initExpression, conditionExpression, stepExpression, multiExpression);
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseWhileExpression()
{
	uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("当 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto conditionExpression = ParseExpression();

		std::unique_ptr<ASTBase> multiExpression = nullptr;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("当 执行表达式期望捕捉 {")))
		{
			multiExpression = ParseMultiExpression();

			GetNextToken();
			return std::make_unique<ASTWhileExpression>(m_Compiler, SourceLocation(tempLineCount), conditionExpression, multiExpression);
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseBreakExpression()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	return std::make_unique<ASTBreakExpression>(m_Compiler, SourceLocation(tempLineCount));
}

std::unique_ptr<ASTBase> Parse::ParseContinueExpression()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	return std::make_unique<ASTContinueExpression>(m_Compiler, SourceLocation(tempLineCount));
}

std::unique_ptr<ASTBase> Parse::ParseReturn()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	auto returnExpression = ParseExpression();
	return std::make_unique<ASTReturn>(m_Compiler, SourceLocation(tempLineCount), returnExpression);
}

std::unique_ptr<ASTBase> Parse::ParseNew()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();

	HazeDefineVariable defineVar;
	defineVar.Type.PrimaryType = GetValueTypeByToken(m_CurrToken);
	GetValueType(defineVar.Type);

	if (ExpectNextTokenIs(HazeToken::Array))
	{
		std::vector<std::unique_ptr<ASTBase>> arraySize;
		while (m_CurrToken == HazeToken::Array)
		{
			if (ExpectNextTokenIs(HazeToken::ArrayDefineEnd))
			{
				GetNextToken();
				break;
			}
			else
			{
				arraySize.push_back(ParseExpression());
				GetNextToken();
			}
		}

		return std::make_unique<ASTNew>(m_Compiler, SourceLocation(tempLineCount), defineVar, std::move(arraySize));
	}
	else
	{
		if (TokenIs(HazeToken::LeftParentheses, HAZE_TEXT("生成表达式 期望 (")))
		{
			if (ExpectNextTokenIs(HazeToken::RightParentheses, HAZE_TEXT("生成表达式 期望 )")))
			{
				GetNextToken();
				return std::make_unique<ASTNew>(m_Compiler, SourceLocation(tempLineCount), defineVar);
			}
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseInc()
{
	uint32 tempLineCount = m_LineCount;

	GetNextToken();
	auto expression = ParseExpression();

	return std::make_unique<ASTInc>(m_Compiler, SourceLocation(tempLineCount), expression, true);
}

std::unique_ptr<ASTBase> Parse::ParseDec()
{
	uint32 tempLineCount = m_LineCount;

	GetNextToken();
	auto expression = ParseExpression();

	return std::make_unique<ASTDec>(m_Compiler, SourceLocation(tempLineCount), expression, true);
}

std::unique_ptr<ASTBase> Parse::ParseThreeOperator(std::unique_ptr<ASTBase> Condition)
{
	uint32 tempLineCount = m_LineCount;
	auto iter = s_HashMap_OperatorPriority.find(HazeToken::ThreeOperatorStart);
	if (iter != s_HashMap_OperatorPriority.end())
	{
		auto conditionExpression = std::move(Condition);

		GetNextToken();
		auto leftExpression = ParseExpression(iter->second);

		if (m_CurrToken != HazeToken::Colon)
		{
			HAZE_LOG_ERR(HAZE_TEXT("三目表达式 需要 : 符号!\n"));
			return nullptr;
		}

		GetNextToken();
		auto rightExpression = ParseExpression(iter->second);

		return std::make_unique<ASTThreeExpression>(m_Compiler, SourceLocation(tempLineCount), conditionExpression, leftExpression, rightExpression);
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseLeftParentheses()
{
	m_LeftParenthesesExpressionCount++;
	GetNextToken();

	std::unique_ptr<ASTBase> expression = nullptr;
	if (IsCanCastToken(m_CurrToken))
	{
		HazeDefineType type;
		type.PrimaryType = GetValueTypeByToken(m_CurrToken);
		GetValueType(type);

		if (ExpectNextTokenIs(HazeToken::RightParentheses))
		{

			if (ExpectNextTokenIs(HazeToken::LeftParentheses))
			{
				expression = ParseExpression();
			}
			else
			{
				expression = ParseUnaryExpression();
			}
			return std::make_unique<ASTCast>(m_Compiler, m_LineCount, type, expression);
		}
	}
	else
	{
		expression = ParseExpression();
		if (m_CurrToken == HazeToken::RightParentheses || m_LeftParenthesesExpressionCount == 0)
		{
			if (m_LeftParenthesesExpressionCount > 0)
			{
				m_LeftParenthesesExpressionCount--;
				GetNextToken();
			}
			return expression;
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("Parse left parentheses error, expect right parentheses\n"));
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParsePointerValue()
{
	uint32 tempLineCount = m_LineCount;
	int level = (int)m_CurrLexeme.length();

	GetNextToken();

	auto expression = ParseExpression(s_HashMap_OperatorPriority.find(HazeToken::PointerValue)->second);

	if (m_CurrToken == HazeToken::RightParentheses)
	{
		if (m_LeftParenthesesExpressionCount > 0)
		{
			m_LeftParenthesesExpressionCount--;
			GetNextToken();
			if (m_CurrToken == HazeToken::Assign)
			{
				GetNextToken();
				auto assignExpression = ParseExpression();
				return std::make_unique<ASTPointerValue>(m_Compiler, SourceLocation(tempLineCount), expression, level, std::move(assignExpression));
			}
		}

		return std::make_unique<ASTPointerValue>(m_Compiler, SourceLocation(tempLineCount), expression, level);
	}
	else if (m_CurrToken == HazeToken::Assign)
	{
		GetNextToken();
		auto assignExpression = ParseExpression();
		return std::make_unique<ASTPointerValue>(m_Compiler, SourceLocation(tempLineCount), expression, level, std::move(assignExpression));
	}
	else
	{
		return std::make_unique<ASTPointerValue>(m_Compiler, SourceLocation(tempLineCount), expression, level);
	}
}

std::unique_ptr<ASTBase> Parse::ParseNeg()
{
	uint32 tempLineCount = m_LineCount;
	bool isNumberNeg = m_CurrToken == HazeToken::Sub;

	GetNextToken();
	auto expression = ParseExpression();

	return std::make_unique<ASTNeg>(m_Compiler, SourceLocation(tempLineCount), expression, isNumberNeg);
}

std::unique_ptr<ASTBase> Parse::ParseNullPtr()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	return std::make_unique<ASTNullPtr>(m_Compiler, SourceLocation(tempLineCount));
}

std::unique_ptr<ASTBase> Parse::ParseGetAddress()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	auto expression = ParseExpression(s_HashMap_OperatorPriority.find(HazeToken::GetAddress)->second);
	m_NeedParseNextStatement = tempLineCount != m_LineCount;

	return std::make_unique<ASTGetAddress>(m_Compiler, SourceLocation(tempLineCount), expression);
}

std::unique_ptr<ASTBase> Parse::ParseLeftBrace()
{
	uint32 tempLineCount = m_LineCount;
	std::vector<std::unique_ptr<ASTBase>> elements;

	GetNextToken();
	while (true)
	{
		elements.push_back(ParseExpression());

		if (m_CurrToken == HazeToken::RightBrace)
		{
			GetNextToken();
			break;
		}
		else if (m_CurrToken == HazeToken::Comma)
		{
			GetNextToken();
		}
	}

	return std::make_unique<ASTInitializeList>(m_Compiler, SourceLocation(tempLineCount), elements);
}

std::unique_ptr<ASTBase> Parse::ParseNot()
{
	int tempLineCount = m_LineCount;

	GetNextToken();
	auto expression = ParseExpression(7000);

	return std::make_unique<ASTNot>(m_Compiler, SourceLocation(tempLineCount), expression);
}

//std::unique_ptr<ASTBase> Parse::ParseOperatorAssign()
//{
//	HazeToken Token = m_CurrToken;
//
//	auto Value = ParseExpression();
//
//	GetNextToken();
//	return std::make_unique<ASTOperetorAssign>(Compiler, SourceLocation(m_LineCount) Token, Value);
//}

std::unique_ptr<ASTBase> Parse::ParseMultiExpression()
{
	std::vector<std::unique_ptr<ASTBase>> expressions;

	GetNextToken();
	while (auto e = ParseExpression())
	{
		expressions.push_back(std::move(e));

		if (TokenIs(HazeToken::RightBrace))
		{
			break;
		}
	}

	return std::make_unique<ASTMultiExpression>(m_Compiler, SourceLocation(m_LineCount), m_StackSectionSignal.top(), expressions);
}

std::unique_ptr<ASTFunctionSection> Parse::ParseFunctionSection()
{
	if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("函数群期望 {")))
	{
		std::vector<std::unique_ptr<ASTFunction>> functions;

		GetNextToken();
		while (m_CurrToken != HazeToken::RightBrace)
		{
			functions.push_back(ParseFunction());
		}

		GetNextToken();
		return std::make_unique<ASTFunctionSection>(m_Compiler,/* SourceLocation(m_LineCount),*/ functions);
	}

	return nullptr;
}

std::unique_ptr<ASTFunction> Parse::ParseFunction(const HAZE_STRING* className)
{
	m_StackSectionSignal.push(HazeSectionSignal::Local);
	uint32 tempLineCount = m_LineCount;

	//获得函数返回类型及是自定义类型时获得类型名字
	HazeDefineType funcType;
	funcType.PrimaryType = GetValueTypeByToken(m_CurrToken);
	GetValueType(funcType);
	
	uint32 startLineCount = m_LineCount;
	HAZE_STRING functionName;

	//获得函数名
	if (ExpectNextTokenIs(HazeToken::Identifier))
	{
		functionName = m_CurrLexeme;
		if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("函数参数定义需要 (")))
		{
			std::vector<std::unique_ptr<ASTBase>> params;

			if (className && !className->empty())
			{
				HazeDefineVariable thisParam;
				thisParam.Name = HAZE_CLASS_THIS;
				thisParam.Type.PrimaryType = HazeValueType::PointerClass;
				thisParam.Type.CustomName = className->c_str();

				params.push_back(std::make_unique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), HazeSectionSignal::Local, thisParam, nullptr));
			}
			
			GetNextToken();

			while (!TokenIs(HazeToken::LeftBrace) && !TokenIs(HazeToken::RightParentheses))
			{
				params.push_back(ParseVariableDefine());
				if (!TokenIs(HazeToken::Comma))
				{
					break;
				}

				GetNextToken();
			}

			if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("函数体需要 {")))
			{
				startLineCount = m_LineCount;
				std::unique_ptr<ASTBase> body = ParseMultiExpression();

				if (TokenIs(HazeToken::RightBrace, HAZE_TEXT("函数体需要 }")))
				{
					m_StackSectionSignal.pop();
					tempLineCount = m_LineCount;

					GetNextToken();
					return std::make_unique<ASTFunction>(m_Compiler, SourceLocation(startLineCount), SourceLocation(tempLineCount),
						m_StackSectionSignal.top(), functionName, funcType, params, body);
				}
			}
		}
	}
	else if (*className == funcType.CustomName)
	{
		//类构造函数
		funcType.PrimaryType = HazeValueType::Void;
		functionName = *className;
		if (m_CurrToken == HazeToken::LeftParentheses)
		{
			std::vector<std::unique_ptr<ASTBase>> params;

			if (className && !className->empty())
			{
				HazeDefineVariable thisParam;
				thisParam.Name = HAZE_CLASS_THIS;
				thisParam.Type.PrimaryType = HazeValueType::PointerClass;
				thisParam.Type.CustomName = className->c_str();

				params.push_back(std::make_unique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount),
					HazeSectionSignal::Local, thisParam, nullptr));
			}

			GetNextToken();

			while (!TokenIs(HazeToken::LeftBrace) && !TokenIs(HazeToken::RightParentheses))
			{
				params.push_back(ParseVariableDefine());
				if (!TokenIs(HazeToken::Comma))
				{
					break;
				}

				GetNextToken();
			}

			if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("类的构造函数体需要 {")))
			{
				startLineCount = m_LineCount;
				std::unique_ptr<ASTBase> body = ParseMultiExpression();

				if (TokenIs(HazeToken::RightBrace, HAZE_TEXT("类的构造函数体需要 }")))
				{
					m_StackSectionSignal.pop();
					tempLineCount = m_LineCount;

					GetNextToken();
					return std::make_unique<ASTFunction>(m_Compiler, SourceLocation(startLineCount), SourceLocation(tempLineCount),
						m_StackSectionSignal.top(), functionName,	funcType, params, body);
				}
			}
		}
		else
		{
			PARSE_ERR_W("类<%s>的构造函数定义错误", className->c_str());
		}
	}

	return nullptr;
}

std::unique_ptr<ASTLibrary> Parse::ParseLibrary()
{
	HazeLibraryType libType = GetHazeLibraryTypeByToken(m_CurrToken);
	GetNextToken();
	HAZE_STRING standardLibraryName = m_CurrLexeme;

	m_StackSectionSignal.push(HazeSectionSignal::Global);

	if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("标准库需要 {")))
	{
		std::vector<std::unique_ptr<ASTClassDefine>> classDefines;
		std::vector<std::unique_ptr<ASTFunctionDefine>> functionDefines;

		GetNextToken();
		while (m_CurrToken == HazeToken::Function || m_CurrToken == HazeToken::Class)
		{
			if (m_CurrToken == HazeToken::Function)
			{
				auto functions = ParseLibrary_FunctionDefine();
				for (auto& iter : functions)
				{
					functionDefines.push_back(std::move(iter));
				}
			}
			else if (m_CurrToken == HazeToken::Class)
			{
				classDefines.push_back(ParseLibrary_ClassDefine());
			}
		}

		m_StackSectionSignal.pop();
		GetNextToken();

		return std::make_unique<ASTLibrary>(m_Compiler, /*SourceLocation(m_LineCount),*/ standardLibraryName, libType, functionDefines, classDefines);
	}

	return nullptr;
}

std::unique_ptr<ASTClassDefine> Parse::ParseLibrary_ClassDefine()
{
	m_CurrParseClass = m_CurrLexeme;
	m_CurrParseClass.clear();
	/*std::vector<std::vector<std::unique_ptr<ASTBase>>> Data;
	std::vector<std::unique_ptr<ASTFunctionDefine>> Function;
	return std::make_unique<ASTClassDefine>(Compiler, SourceLocation(m_LineCount) CurrLexeme, Data, Function);*/
	return nullptr;
}

std::vector<std::unique_ptr<ASTFunctionDefine>> Parse::ParseLibrary_FunctionDefine()
{
	std::vector<std::unique_ptr<ASTFunctionDefine>> functionDefines;

	if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("标准库函数群需要 {")))
	{
		GetNextToken();
		while (m_CurrToken != HazeToken::RightBrace)
		{
			m_StackSectionSignal.push(HazeSectionSignal::Local);

			//获得函数返回类型及是自定义类型时获得类型名字
			HazeDefineType funcType;
			funcType.PrimaryType = GetValueTypeByToken(m_CurrToken);
			GetValueType(funcType);

			//获得函数名
			if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("库函数命名错误")))
			{
				HAZE_STRING functionName = m_CurrLexeme;
				if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("函数参数定义需要 (")))
				{
					std::vector<std::unique_ptr<ASTBase>> params;

					GetNextToken();

					while (!TokenIs(HazeToken::LeftBrace) && !TokenIs(HazeToken::RightParentheses))
					{
						params.push_back(ParseVariableDefine());
						if (!TokenIs(HazeToken::Comma) || params.back()->GetDefine().Type.PrimaryType == HazeValueType::MultiVariable)
						{
							break;
						}

						GetNextToken();
					}

					m_StackSectionSignal.pop();
					functionDefines.push_back(std::make_unique<ASTFunctionDefine>(m_Compiler, /*SourceLocation(m_LineCount),*/ functionName, funcType, params));
					GetNextToken();
				}
			}
		}

		GetNextToken();
		//return std::make_unique<ASTStandardLibrary>(Compiler, SourceLocation(m_LineCount) StandardLibraryName, Vector_FunctionDefine);
	}

	return functionDefines;
}

std::unique_ptr<ASTBase> Parse::ParseImportModule()
{
	if (ExpectNextTokenIs(HazeToken::Identifier))
	{
		HAZE_STRING name = m_CurrLexeme;
		return std::make_unique<ASTImportModule>(m_Compiler, SourceLocation(m_LineCount), name);
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("解析错误: 引入模块<%s>错误! <%s>文件<%d>行!\n"), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
	}

	return nullptr;
}

std::unique_ptr<ASTClass> Parse::ParseClass()
{
	if (ExpectNextTokenIs(HazeToken::Identifier))
	{
		m_CurrParseClass = m_CurrLexeme;
		HAZE_STRING name = m_CurrLexeme;
		std::vector<HAZE_STRING> parentClasses;

		GetNextToken();
		if (TokenIs(HazeToken::Colon))
		{
			if (ExpectNextTokenIs(HazeToken::CustomClass))
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
						HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n"), name.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
					}
				}
			}
			else
			{
				HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n"), name.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
			}
		}

		if (TokenIs(HazeToken::LeftBrace))
		{
			m_StackSectionSignal.push(HazeSectionSignal::Class);

			std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> classDatas;
			std::unique_ptr<ASTClassFunctionSection> classFunctions;

			GetNextToken();
			while (m_CurrToken == HazeToken::m_ClassDatas || m_CurrToken == HazeToken::Function)
			{
				if (m_CurrToken == HazeToken::m_ClassDatas)
				{
					if (classDatas.size() == 0)
					{
						classDatas = ParseClassData();
					}
					else
					{
						HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类中相同的区域<%s>只能存在一种! <%s>文件<%d>行!"), name.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
					}
				}
				else if (m_CurrToken == HazeToken::Function)
				{
					classFunctions = ParseClassFunction(name);
				}
			}

			m_StackSectionSignal.pop();

			GetNextToken();

			m_CurrParseClass.clear();
			return std::make_unique<ASTClass>(m_Compiler, /*SourceLocation(m_LineCount),*/ name, parentClasses, classDatas, classFunctions);
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类名<%s>错误! <%s>文件<%d>行!"), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
	}

	return nullptr;
}

std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> Parse::ParseClassData()
{
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> classDatas;

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

	if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("类数据需要 {")))
	{
		GetNextToken();

		while (m_CurrToken == HazeToken::ClassPublic || m_CurrToken == HazeToken::ClassPrivate || m_CurrToken == HazeToken::ClassProtected)
		{
			if (m_CurrToken == HazeToken::ClassPublic)
			{
				if (!HasData(classDatas, HazeDataDesc::ClassMember_Local_Public))
				{
					classDatas.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Public,
						std::move(std::vector<std::unique_ptr<ASTBase>>{}) })));

					GetNextToken();
					GetNextToken();
					while (m_CurrToken != HazeToken::RightBrace)
					{
						classDatas.back().second.push_back(ParseVariableDefine());
					}

					GetNextToken();
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类的公有区域只能定义一次! <%s>文件<%d>行!\n"), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
			else if (m_CurrToken == HazeToken::ClassPrivate)
			{
				if (!HasData(classDatas, HazeDataDesc::ClassMember_Local_Private))
				{
					classDatas.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Private,
						std::move(std::vector<std::unique_ptr<ASTBase>>{}) })));

					GetNextToken();
					GetNextToken();
					while (m_CurrToken != HazeToken::RightBrace)
					{
						classDatas.back().second.push_back(ParseVariableDefine());
					}

					GetNextToken();
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类的私有区域只能定义一次! <%s>文件<%d>行!\n"), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
			else if (m_CurrToken == HazeToken::ClassProtected)
			{
				if (!HasData(classDatas, HazeDataDesc::ClassMember_Local_Protected))
				{
					classDatas.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Protected,
						std::move(std::vector<std::unique_ptr<ASTBase>>{}) })));

					GetNextToken();
					GetNextToken();
					while (m_CurrToken != HazeToken::RightBrace)
					{
						classDatas.back().second.push_back(ParseVariableDefine());
					}

					GetNextToken();
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类的保护区域只能定义一次! <%s>文件<%d>行!\n"), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
		}

		if (m_CurrToken != HazeToken::RightBrace)
		{
			HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类需要 }! <%s>文件<%d>行!\n"), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
		}

		GetNextToken();
	}

	return classDatas;
}

std::unique_ptr<ASTClassFunctionSection> Parse::ParseClassFunction(const HAZE_STRING& className)
{
	if (ExpectNextTokenIs(HazeToken::LeftBrace, HAZE_TEXT("类函数定义需要 {")))
	{
		GetNextToken();

		std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>> classFunctions;
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

		while (m_CurrToken == HazeToken::ClassPublic || m_CurrToken == HazeToken::ClassPrivate || m_CurrToken == HazeToken::ClassProtected)
		{
			if (m_CurrToken == HazeToken::ClassPublic)
			{
				if (!(HasData(classFunctions, HazeDataDesc::ClassFunction_Local_Public)))
				{
					classFunctions.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Public,
						std::move(std::vector<std::unique_ptr<ASTFunction>>{}) })));

					GetNextToken();
					GetNextToken();
					while (m_CurrToken != HazeToken::RightBrace)
					{
						classFunctions.back().second.push_back(ParseFunction(&className));
					}

					GetNextToken();
				}
			}
			else if (m_CurrToken == HazeToken::ClassPrivate)
			{
				if (!(HasData(classFunctions, HazeDataDesc::ClassFunction_Local_Private)))
				{
					classFunctions.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Private,
						std::move(std::vector<std::unique_ptr<ASTFunction>>{}) })));

					GetNextToken();
					GetNextToken();

					while (m_CurrToken != HazeToken::RightBrace)
					{
						classFunctions.back().second.push_back(ParseFunction(&className));
					}

					GetNextToken();
				}
			}
			else if (m_CurrToken == HazeToken::ClassProtected)
			{
				if (!(HasData(classFunctions, HazeDataDesc::ClassFunction_Local_Protected)))
				{
					classFunctions.push_back(std::move(
						std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Protected,
						std::move(std::vector<std::unique_ptr<ASTFunction>>{}) })));

					GetNextToken();
					GetNextToken();

					while (m_CurrToken != HazeToken::RightBrace)
					{
						classFunctions.back().second.push_back(ParseFunction(&className));
					}

					GetNextToken();
				}
			}
		}

		if (m_CurrToken != HazeToken::RightBrace)
		{
			PARSE_ERR_W("类<%s>函数需要 }", className.c_str());
		}

		GetNextToken();
		return std::make_unique<ASTClassFunctionSection>(m_Compiler, /*SourceLocation(m_LineCount),*/ classFunctions);
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseArrayLength()
{
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("获得数组长度需要 （")))
	{
		GetNextToken();
		auto variable = ParseExpression();

		if (TokenIs(HazeToken::RightParentheses, HAZE_TEXT("获得数组长度需要 )")))
		{
			GetNextToken();
			return std::make_unique<ASTArrayLength>(m_Compiler, m_LineCount, variable);
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseSizeOf()
{
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, HAZE_TEXT("获得字节大小需要 （")))
	{
		GetNextToken();
		auto variable = ParseExpression();

		if (ExpectNextTokenIs(HazeToken::RightParentheses, HAZE_TEXT("获得字节大小需要 )")))
		{
			GetNextToken();
			return std::make_unique<ASTSizeOf>(m_Compiler, m_LineCount, variable);
		}
	}

	return nullptr;
}

std::unique_ptr<ASTBase> Parse::ParseEnum()
{
	return std::unique_ptr<ASTBase>();
}

void Parse::ParseTemplate()
{
	if (ExpectNextTokenIs(HazeToken::Less))
	{
		std::vector<HAZE_STRING> templateTypes;
		do
		{
			if (ExpectNextTokenIs(HazeToken::TypeName))
			{
				if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("模板类型名定义错误")))
				{
					templateTypes.push_back(m_CurrLexeme);
				}
			}
		} while (TokenIs(HazeToken::Comma));

		if (ExpectNextTokenIs(HazeToken::Greater))
		{
			if (ExpectNextTokenIs(HazeToken::Class))
			{
				if (ExpectNextTokenIs(HazeToken::Identifier, HAZE_TEXT("模板类名定义错误")))
				{
					HAZE_STRING templateClassName = m_CurrLexeme;
					const HAZE_CHAR* start = m_CurrCode;
					uint32 line = m_LineCount;

					if (ExpectNextTokenIs(HazeToken::Colon))
					{
						//暂时不支持继承模板类
						if (ExpectNextTokenIs(HazeToken::CustomClass))
						{
							while (TokenIs(HazeToken::CustomClass))
							{
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
									HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n"), templateClassName.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
								}

								line = m_LineCount;
							}
						}
						else
						{
							HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n"), templateClassName.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
						}
					}
					
					if (TokenIs(HazeToken::LeftBrace))
					{
						std::vector<uint32> stack(1);
						while (stack.size() > 0)
						{
							if (ExpectNextTokenIs(HazeToken::LeftBrace))
							{
								stack.push_back(1);
							}
							else if (TokenIs(HazeToken::RightBrace))
							{
								stack.pop_back();
							}
						}

						HAZE_STRING templateText(start, m_CurrCode);
						m_Compiler->GetCurrModule()->StartCacheTemplate(templateClassName, line, templateText, templateTypes);
					}
				}
			}
			else
			{
				//模板函数
			}

			GetNextToken();
		}
	}
}

std::unique_ptr<ASTTemplateBase> Parse::ParseTemplateClass(std::vector<HAZE_STRING>& templateTypes)
{
	m_CurrParseClass = m_CurrLexeme;
	HAZE_STRING name = m_CurrLexeme;
	std::vector<HAZE_STRING> parentClasses;

	GetNextToken();
	if (TokenIs(HazeToken::Colon))
	{
		//暂时不支持继承模板类
		if (ExpectNextTokenIs(HazeToken::CustomClass))
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
					HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n"), name.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n"), name.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
		}
	}

	if (TokenIs(HazeToken::LeftBrace))
	{
		m_StackSectionSignal.push(HazeSectionSignal::Class);

		std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> classDatas;
		std::unique_ptr<ASTClassFunctionSection> classFunctions;

		GetNextToken();
		while (m_CurrToken == HazeToken::m_ClassDatas || m_CurrToken == HazeToken::Function)
		{
			if (m_CurrToken == HazeToken::m_ClassDatas)
			{
				if (classDatas.size() == 0)
				{
					classDatas = ParseClassData();
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("解析错误: 类中相同的区域<%s>只能存在一种! <%s>文件<%d>行!"), name.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
			else if (m_CurrToken == HazeToken::Function)
			{
				classFunctions = ParseClassFunction(name);
			}
		}

		m_StackSectionSignal.pop();

		GetNextToken();

		m_CurrParseClass.clear();
		return nullptr; //return std::make_unique<ASTTemplateClass>(m_Compiler, /*SourceLocation(m_LineCount),*/ name, parentClasses, classDatas, classFunctions);
	}
}
std::unique_ptr<ASTTemplateBase> Parse::ParseTemplateFunction(std::vector<HAZE_STRING>& templateTypes)
{
	return std::unique_ptr<ASTTemplateBase>();
}

bool Parse::ExpectNextTokenIs(HazeToken token, const HAZE_CHAR* errorInfo)
{
	HazeToken NextToken = GetNextToken();
	if (token != NextToken)
	{
		if (errorInfo)
		{
			PARSE_ERR_W("%s", errorInfo);
		}
		return false;
	}

	return true;
}

bool Parse::TokenIs(HazeToken token, const HAZE_CHAR* errorInfo)
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

bool Parse::IsHazeSignalToken(const HAZE_CHAR* hChar, const HAZE_CHAR*& outChar, uint32 charSize)
{
	static std::unordered_set<HAZE_STRING> s_HashSet_TokenText =
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

	static HAZE_STRING s_WS;

	s_WS.resize(charSize);
	memcpy(s_WS.data(), hChar, sizeof(HAZE_CHAR) * charSize);

	auto iter = s_HashSet_TokenText.find(s_WS);
	if (iter != s_HashSet_TokenText.end())
	{
		outChar = iter->c_str();
	}

	return iter != s_HashSet_TokenText.end();
}

bool Parse::IsPointerOrRef(const HAZE_STRING& str, HazeToken& outToken)
{
	HAZE_STRING typeName = str.substr(0, str.length() - 1);
	HAZE_STRING pointerChar = str.substr(str.length() - 1, 1);

	if (pointerChar == MUL_STR)
	{
		auto it = s_HashMap_Token.find(typeName);
		if (it != s_HashMap_Token.end())
		{
			if (IsHazeDefaultTypeAndVoid(GetValueTypeByToken(it->second)))
			{
				outToken = HazeToken::PointerBase;
				return true;
			}
		}
		else
		{
			if (m_Compiler->IsClass(typeName))
			{
				outToken = HazeToken::PointerClass;
				return true;
			}

			if (m_IsParseTemplate)
			{
				for (size_t i = 0; i < m_TemplateTypes->size(); i++)
				{
					if (m_TemplateTypes->at(i) == typeName)
					{
						auto& realType = m_TemplateRealTypes->at(i);
						if (realType.HasCustomName())
						{
							if (m_Compiler->IsClass(realType.CustomName))
							{
								outToken = HazeToken::Identifier;
								return true;
							}
						}
						else if (IsPointerType(realType.PrimaryType))
						{
							outToken = HazeToken::Identifier;
							return true;
						}
						else
						{
							if (IsHazeDefaultTypeAndVoid(realType.PrimaryType))
							{
								outToken = HazeToken::Identifier;
								return true;
							}
							else
							{
								HAZE_LOG_ERR_W("生成模板时,类型匹配错误\n");
								return false;
							}
						}
					}
				}
			}

			for (size_t i = 0; i < str.length() - 1; i++)
			{
				if (str.substr(i, 1) != TOKEN_MUL)
				{
					return false;
				}
			}

			if (str.length() > HazeCompiler::GetMaxPointerLevel())
			{
				HAZE_LOG_ERR(HAZE_TEXT("Haze max 3 level pointer pointer !\n"));
				return false;
			}

			outToken = HazeToken::PointerPointer;
			return true;
		}
	}
	else if (pointerChar == TOKEN_BIT_AND)
	{
		auto it = s_HashMap_Token.find(typeName);
		if (it != s_HashMap_Token.end())
		{
			if (IsHazeDefaultTypeAndVoid(GetValueTypeByToken(it->second)))
			{
				outToken = HazeToken::ReferenceBase;
				return true;
			}
		}
		else
		{
			if (m_Compiler->IsClass(typeName))
			{
				outToken = HazeToken::ReferenceClass;
				return true;
			}

			if (m_IsParseTemplate)
			{
				for (size_t i = 0; i < m_TemplateTypes->size(); i++)
				{
					if (m_TemplateTypes->at(i) == typeName)
					{
						auto& realType = m_TemplateRealTypes->at(i);
						if (realType.HasCustomName())
						{
							if (m_Compiler->IsClass(realType.CustomName))
							{
								outToken = HazeToken::ReferenceClass;
								return true;
							}
						}
						else if (realType.SecondaryType != HazeValueType::Void)
						{
							HAZE_LOG_ERR_W("生成模板时,类型匹配错误\n");
							return false;
						}
						else
						{
							if (IsHazeDefaultTypeAndVoid(realType.PrimaryType))
							{
								outToken = HazeToken::ReferenceBase;
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

void Parse::GetValueType(HazeDefineType& inType)
{
	switch (m_CurrToken)
	{
	case HazeToken::None:
		break;
	case HazeToken::Identifier:
		if (m_IsParseTemplate)
		{
			GetTemplateRealValueType(m_CurrLexeme, inType);
		}
		break;
	case HazeToken::MultiVariable:
		m_DefineVariable.Name = HAZE_MULTI_PARAM_NAME;
		break;
	case HazeToken::CustomClass:
	{
		inType.CustomName = m_CurrLexeme;
		if (m_IsParseTemplate)
		{
			GetTemplateClassName(inType.CustomName, *m_TemplateRealTypes);
		}
	}
		break;
	case HazeToken::ReferenceBase:
	case HazeToken::PointerBase:
	{
		inType.SecondaryType = GetPointerBaseType(m_CurrLexeme);
		inType.CustomName.clear();
	}
		break;
	case HazeToken::ReferenceClass:
	case HazeToken::PointerClass:
	{
		inType.SecondaryType = HazeValueType::Class;
		inType.CustomName = GetPointerClassType(m_CurrLexeme);
	}
		break;
	case HazeToken::PointerPointer:
	{
		inType.SecondaryType = GetPointerBaseType(m_CurrLexeme);
		if (IsNoneType(inType.SecondaryType))
		{
			inType.CustomName = m_CurrLexeme;
		}
	}
		break;
	case HazeToken::Void:
	case HazeToken::Bool:
	case HazeToken::Byte:
	case HazeToken::Char:
	case HazeToken::Int:
	case HazeToken::Float:
	case HazeToken::Long:
	case HazeToken::Double:
	case HazeToken::UnsignedInt:
	case HazeToken::UnsignedLong:
		return;
	default:
		PARSE_ERR_W("获得变量类型错误");
		break;
	}
}

void Parse::ResetArrayVariableType(HazeDefineType& inType)
{
	switch (inType.PrimaryType)
	{
	case HazeValueType::Bool:
	case HazeValueType::Byte:
	case HazeValueType::Char:
	case HazeValueType::Int:
	case HazeValueType::Float:
	case HazeValueType::Long:
	case HazeValueType::Double:
	case HazeValueType::UnsignedInt:
	case HazeValueType::UnsignedLong:
	{
		inType.SecondaryType = inType.PrimaryType;
		inType.PrimaryType = HazeValueType::ArrayBase;
	}
		break;
	case HazeValueType::Class:
	{
		inType.PrimaryType = HazeValueType::ArrayClass;
	}
		break;
	case HazeValueType::PointerBase:
	{
		inType.PrimaryType = HazeValueType::ArrayPointer;
	}
		break;
	default:
		PARSE_ERR_W("数组变量类型定义错误");
		break;
	}
}

void Parse::GetTemplateRealValueType(const HAZE_STRING& str, HazeDefineType& inType)
{
	if (m_IsParseTemplate)
	{
		HAZE_STRING pointerChar = str.substr(str.length() - 1, 1);

		if (pointerChar == MUL_STR)
		{
			int pointerLevel = 1;

			HAZE_STRING typeName = str.substr(0, str.length() - 1);
			while (pointerLevel <= HazeCompiler::GetMaxPointerLevel())
			{
				for (size_t i = 0; i < m_TemplateTypes->size(); i++)
				{
					if (m_TemplateTypes->at(i) == typeName)
					{
						auto& templateType = m_TemplateRealTypes->at(i);
						if (IsPointerPointer(templateType.PrimaryType))
						{
							pointerLevel += 2;
						}
						else if (IsPointerType(templateType.PrimaryType))
						{
							pointerLevel++;
						}

						if (pointerLevel > HazeCompiler::GetMaxPointerLevel())
						{
							PARSE_ERR_W("查找匹配的模板类型<%s>错误，最多支持<%d>层指针，但是为<%d>层指针", str.c_str(), HazeCompiler::GetMaxPointerLevel(), pointerLevel);
							return;
						}

						if (pointerLevel == 2)
						{
							inType.PrimaryType = HazeValueType::PointerPointer;
							inType.SecondaryType = templateType.SecondaryType;
							inType.CustomName = templateType.CustomName;
						}
						else
						{
							inType.PrimaryType = HazeValueType::PointerPointer;
							inType.SecondaryType = templateType.PrimaryType;
							inType.CustomName = templateType.CustomName;
						}
						return;
					}
				}

				pointerLevel++;
				pointerChar = str.substr(str.length() - pointerLevel, 1);
				if (pointerChar != MUL_STR)
				{
					PARSE_ERR_W("查找匹配的模板类型<%s>错误", str.c_str());
					return;
				}
			}
			
		}
		else
		{
			for (size_t i = 0; i < m_TemplateTypes->size(); i++)
			{
				if (m_TemplateTypes->at(i) == str)
				{
					inType = m_TemplateRealTypes->at(i);
					return;
				}
			}

			HAZE_LOG_ERR_W("模板未能找到<%s>的真正类型!\n", str.c_str());
		}
	}

	return;
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
//		m_DefineVariable.Type.CustomName = HAZE_TEXT("");
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

void Parse::IncLineCount(bool insert)
{
	m_LineCount++;

#if HAZE_DEBUG_ENABLE
	HAZE_CHAR code[20];
	memcpy(code, m_CurrCode + 1, sizeof(code));
	code[19] = '\0';

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