#include "HazePch.h"
#include "HazeVM.h"
#include "Parse.h"

#include "ASTBase.h"
#include "ASTClass.h"
#include "ASTEnum.h"
#include "ASTFunction.h"
#include "ASTLibrary.h"
#include "ASTTemplateClass.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"

#include "HazeLogDefine.h"
#include <cstdarg>

#include "HazeTokenText.h"

static HString MUL_STR = TOKEN_MUL;

//因为会出现嵌套解析同一种AST的情况，所以需要记录不同的行

static HashMap<HString, HazeToken> s_HashMap_Token =
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

	{ TOKEN_ARRAY_START, HazeToken::V_Array },
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

	{ TOKEN_TWO_COLON, HazeToken::TwoColon },
};

const HashMap<HString, HazeToken>& GetHashMap_Token()
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

	{ HazeToken::PointerValue, 7000 },
	{ HazeToken::GetAddress, 7000 },

	//{ HazeToken::Not, 7000 },
	{ HazeToken::Inc, 9000 },
	{ HazeToken::Dec, 9000 },

	//{ HazeToken::LeftParentheses, 10000 },
};

static HazeValueType GetPointerBaseType(const HString& str)
{
	HazeToken token = s_HashMap_Token.find(str.substr(0, str.length() - 1))->second;
	return GetValueTypeByToken(token);
}

static HString GetPointerClassType(const HString& str)
{
	return str.substr(0, str.length() - 1);
}

//static void GetParseType(HazeDefineType& Type, const HString& Str)
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
	std::ifstream s;
	s.open("");
	std::string s1;
	std::getline(s, s1);
}

void Parse::InitializeFile(const HString& filePath)
{
	HAZE_BINARY_IFSTREAM fs(filePath);
	fs.imbue(std::locale("chs"));

	std::string content(std::istreambuf_iterator<char>(fs), {});
	content = UTF8_2_GB2312(content.c_str());
	m_CodeText = String2WString(content);

	m_CurrCode = m_CodeText.c_str();
	fs.close();
}

void Parse::InitializeString(const HString& str, uint32 startLine)
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

void Parse::ParseTemplateContent(const HString& moduleName, const HString& templateName, const V_Array<HString>& templateTypes, const V_Array<HazeDefineType>& templateRealTypes)
{
	m_Compiler->MarkParseTemplate(true, &moduleName);
	m_IsParseTemplate = true;
	m_TemplateTypes = &templateTypes;
	m_TemplateRealTypes = &templateRealTypes;

	m_CurrParseClass = templateName;
	V_Array<HString> parentClasses;
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
					HAZE_LOG_ERR_W("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n", templateName.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
		}
		else
		{
			HAZE_LOG_ERR_W("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n", templateName.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
		}
	}

	if (TokenIs(HazeToken::LeftBrace))
	{
		m_StackSectionSignal.push(HazeSectionSignal::Class);

		V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> classDatas;
		Unique<ASTClassFunctionSection> classFunctions;

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
					HAZE_LOG_ERR_W("解析错误: 类中相同的区域<%s>只能存在一种! <%s>文件<%d>行!", templateName.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
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

		HString className = templateName;
		MakeUnique<ASTClass>(m_Compiler, /*SourceLocation(m_LineCount),*/ className,
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
	m_CurrLexeme.clear();
	const HChar* signal;
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
				static HString tempString;
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
			else if (HString(signal) == TOKEN_ARRAY_START)
			{
				if (m_CurrLexeme.empty())
				{
					m_CurrLexeme += *m_CurrCode++;
					m_CurrToken = HazeToken::V_Array;
					return m_CurrToken;
				}
			}
			else if (m_CurrToken == HazeToken::V_Array && HString(signal) == TOKEN_ARRAY_END)
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

	static HString s_CommentStr;
	s_CommentStr = *m_CurrCode;
	if (m_CurrLexeme == HAZE_SINGLE_COMMENT)
	{
		while (s_CommentStr != H_TEXT("\n"))
		{
			m_CurrCode++;
			s_CommentStr = *m_CurrCode;

			if (HString(m_CurrCode) == H_TEXT(""))
			{
				break;
			}
		}
		return GetNextToken();
	}
	else if (m_CurrLexeme == HAZE_MULTI_COMMENT_START)
	{
		s_CommentStr.resize(2);
		while (s_CommentStr != HAZE_MULTI_COMMENT_END)
		{
			m_CurrCode++;
			memcpy(s_CommentStr.data(), m_CurrCode, sizeof(HChar) * 2);

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

Unique<ASTBase> Parse::HandleParseExpression()
{
	return ParseExpression();
}

Unique<ASTBase> Parse::ParseExpression(int prec)
{
	if (m_NeedParseNextStatement)
	{
		m_NeedParseNextStatement = false;
	}

	Unique<ASTBase> left = ParseUnaryExpression();

	if (left)
	{
		return ParseBinaryOperateExpression(prec, std::move(left));
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseUnaryExpression()
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

Unique<ASTBase> Parse::ParseBinaryOperateExpression(int prec, Unique<ASTBase> left)
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
		Unique<ASTBase> right = nullptr;
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
			return MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), opToken, left, right);
		}

		if (it->second < nextPrec->second)
		{
			right = ParseBinaryOperateExpression(it->second + 1, std::move(right));
			if (!right)
			{
				return nullptr;
			}

			left = MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), opToken, left, right);
		}
		else if (nextPrec == s_HashMap_OperatorPriority.find(HazeToken::ThreeOperatorStart))
		{
			left = ParseThreeOperator(right ? MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), opToken, left, right) : std::move(left));
		}
		else
		{
			left = MakeUnique<ASTBinaryExpression>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), opToken, left, right);
		}
	}
}

Unique<ASTBase> Parse::ParsePrimary()
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

Unique<ASTBase> Parse::ParseIdentifer()
{
	uint32 tempLineCount = m_LineCount;
	Unique<ASTBase> ret = nullptr;
	HString identiferName = m_CurrLexeme;
	V_Array<Unique<ASTBase>> indexExpression;

	if (GetNextToken() == HazeToken::LeftParentheses && m_LineCount == tempLineCount)
	{
		//函数调用
		V_Array<Unique<ASTBase>> params;
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
		return MakeUnique<ASTFunctionCall>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), identiferName, params);
	}
	else if (m_CurrToken == HazeToken::V_Array)
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
			while (m_CurrToken == HazeToken::V_Array)
			{
				GetNextToken();
				indexExpression.push_back(ParseExpression());

				GetNextToken();
			}
		}
		

		ret = MakeUnique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), identiferName, indexExpression);
	}
	else if (m_CurrToken == HazeToken::TwoColon)
	{
		if (ExpectNextTokenIs(HazeToken::Identifier)) 
		{
			HString nameSpace = m_CurrLexeme;
			GetNextToken();
			return MakeUnique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
				identiferName, indexExpression, nameSpace);
		}
	}
	else
	{
		ret = MakeUnique<ASTIdentifier>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), identiferName, indexExpression);
	}

	return ret;
}

Unique<ASTBase> Parse::ParseVariableDefine()
{
	uint32 tempLineCount = m_LineCount;
	m_DefineVariable.Name.clear();
	m_DefineVariable.Type.Reset();

	m_DefineVariable.Type.PrimaryType = GetValueTypeByToken(m_CurrToken);

	int pointerLevel = 1;
	GetValueType(m_DefineVariable.Type);
	GetNextToken();

	V_Array<Unique<ASTBase>> arraySize;
	if (TokenIs(HazeToken::V_Array))
	{
		ResetArrayVariableType(m_DefineVariable.Type);

		while (m_CurrToken == HazeToken::V_Array)
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
			Unique<ASTBase> expression = ParseExpression();

			return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount),m_StackSectionSignal.top(),
				m_DefineVariable, std::move(expression), std::move(arraySize), pointerLevel);
		}
		else if ((m_CurrToken == HazeToken::RightParentheses || m_CurrToken == HazeToken::Comma) && m_StackSectionSignal.top() == HazeSectionSignal::Local)
		{
			//函数调用
			return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
				m_DefineVariable, nullptr);
		}
		else if (m_DefineVariable.Type.PrimaryType == HazeValueType::Class)
		{
			if (m_CurrToken == HazeToken::LeftParentheses)
			{
				V_Array<Unique<ASTBase>>& params = arraySize;

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

				GetNextToken();
				return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(),
					m_DefineVariable, nullptr, std::move(params), pointerLevel);
			}
			else if (isTemplateVar)
			{
				V_Array<HazeDefineType> templateTypes;
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
					if (ExpectNextTokenIs(HazeToken::Identifier, H_TEXT("模板类对象命名错误")))
					{
						m_DefineVariable.Name = m_CurrLexeme;
						if (ExpectNextTokenIs(HazeToken::LeftParentheses) && ExpectNextTokenIs(HazeToken::RightParentheses))
						{
							GetNextToken();
							return  MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount),
								m_StackSectionSignal.top(), m_DefineVariable, nullptr, std::move(arraySize), pointerLevel, &templateTypes);
						}
					}
				}
			}
			else
			{
				HAZE_LOG_ERR_W("解析错误 类对象定义需要括号\"(\" <%s>文件<%d>行!!\n", m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
			}
		}
		else
		{
			if (IsPointerType(m_DefineVariable.Type.PrimaryType))
			{
				PARSE_ERR_W("指针类型需要初始化");
			}

			return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), m_DefineVariable, nullptr, std::move(arraySize), pointerLevel);
		}
	}
	else if (m_CurrToken == HazeToken::LeftParentheses)
	{
		//函数指针或数组指针
		if (ExpectNextTokenIs(HazeToken::Mul) && ExpectNextTokenIs(HazeToken::Identifier, H_TEXT("函数指针或者数组指针需要一个正确的名称")))
		{
			V_Array<HazeDefineType> paramTypes;
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

						Unique<ASTBase> expression = ParseExpression();

						return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), m_StackSectionSignal.top(), m_DefineVariable, std::move(expression), std::move(arraySize), pointerLevel);
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

	return MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(m_LineCount), m_StackSectionSignal.top(), m_DefineVariable, nullptr, std::move(arraySize), pointerLevel);;
}

Unique<ASTBase> Parse::ParseStringText()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	HString text = m_CurrLexeme;

	GetNextToken();
	return MakeUnique<ASTStringText>(m_Compiler, SourceLocation(tempLineCount), text);
}

Unique<ASTBase> Parse::ParseBoolExpression()
{
	uint32 tempLineCount = m_LineCount;
	HazeValue value;
	value.Value.Bool = m_CurrLexeme == TOKEN_TRUE;

	GetNextToken();
	return MakeUnique<ASTBool>(m_Compiler, SourceLocation(tempLineCount), value);
}

Unique<ASTBase> Parse::ParseNumberExpression()
{
	uint32 tempLineCount = m_LineCount;
	HazeValue value;
	HazeValueType type = GetNumberDefaultType(m_CurrLexeme);

	StringToHazeValueNumber(m_CurrLexeme, type, value);

	GetNextToken();
	return MakeUnique<ASTNumber>(m_Compiler, SourceLocation(tempLineCount), type, value);
}

Unique<ASTBase> Parse::ParseIfExpression(bool recursion)
{
	uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("若 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto conditionExpression = ParseExpression();

		Unique<ASTBase> ifMultiExpression = nullptr;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("若 执行表达式期望捕捉 { ")))
		{
			ifMultiExpression = ParseMultiExpression();
			GetNextToken();
		}

		Unique<ASTBase> elseExpression = nullptr;

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

		return MakeUnique<ASTIfExpression>(m_Compiler, SourceLocation(tempLineCount), conditionExpression, ifMultiExpression, elseExpression);
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseForExpression()
{
	uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("循环 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto initExpression = ParseExpression();

		GetNextToken();
		auto conditionExpression = ParseExpression();

		GetNextToken();
		auto stepExpression = ParseExpression();

		if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("循环 表达式块期望捕捉 {")))
		{
			auto multiExpression = ParseMultiExpression();

			GetNextToken();
			return MakeUnique<ASTForExpression>(m_Compiler, SourceLocation(tempLineCount), initExpression, conditionExpression, stepExpression, multiExpression);
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseWhileExpression()
{
	uint32 tempLineCount = m_LineCount;
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("当 表达式期望捕捉 (")))
	{
		GetNextToken();
		auto conditionExpression = ParseExpression();

		Unique<ASTBase> multiExpression = nullptr;
		if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("当 执行表达式期望捕捉 {")))
		{
			multiExpression = ParseMultiExpression();

			GetNextToken();
			return MakeUnique<ASTWhileExpression>(m_Compiler, SourceLocation(tempLineCount), conditionExpression, multiExpression);
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseBreakExpression()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	return MakeUnique<ASTBreakExpression>(m_Compiler, SourceLocation(tempLineCount));
}

Unique<ASTBase> Parse::ParseContinueExpression()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	return MakeUnique<ASTContinueExpression>(m_Compiler, SourceLocation(tempLineCount));
}

Unique<ASTBase> Parse::ParseReturn()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	auto returnExpression = ParseExpression();
	return MakeUnique<ASTReturn>(m_Compiler, SourceLocation(tempLineCount), returnExpression);
}

Unique<ASTBase> Parse::ParseNew()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();

	HazeDefineVariable defineVar;
	defineVar.Type.PrimaryType = GetValueTypeByToken(m_CurrToken);
	GetValueType(defineVar.Type);

	V_Array<Unique<ASTBase>> arraySize;
	if (ExpectNextTokenIs(HazeToken::V_Array))
	{
		while (m_CurrToken == HazeToken::V_Array)
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

		return MakeUnique<ASTNew>(m_Compiler, SourceLocation(tempLineCount), defineVar, std::move(arraySize));
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
				if (!TokenIs(HazeToken::Comma))
				{
					break;
				}

				GetNextToken();
			}
			
			GetNextToken();
			return MakeUnique<ASTNew>(m_Compiler, SourceLocation(tempLineCount), defineVar, 
				std::move(arraySize), std::move(params));
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseInc()
{
	uint32 tempLineCount = m_LineCount;

	GetNextToken();
	auto expression = ParseExpression();

	return MakeUnique<ASTInc>(m_Compiler, SourceLocation(tempLineCount), expression, true);
}

Unique<ASTBase> Parse::ParseDec()
{
	uint32 tempLineCount = m_LineCount;

	GetNextToken();
	auto expression = ParseExpression();

	return MakeUnique<ASTDec>(m_Compiler, SourceLocation(tempLineCount), expression, true);
}

Unique<ASTBase> Parse::ParseThreeOperator(Unique<ASTBase> Condition)
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
			HAZE_LOG_ERR_W("三目表达式 需要 : 符号!\n");
			return nullptr;
		}

		GetNextToken();
		auto rightExpression = ParseExpression(iter->second);

		return MakeUnique<ASTThreeExpression>(m_Compiler, SourceLocation(tempLineCount), conditionExpression, leftExpression, rightExpression);
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
			return MakeUnique<ASTCast>(m_Compiler, m_LineCount, type, expression);
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
			PARSE_ERR_W("解析 ( 错误, 期望 ） \n");
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParsePointerValue()
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
				return MakeUnique<ASTPointerValue>(m_Compiler, SourceLocation(tempLineCount), expression, level, std::move(assignExpression));
			}
		}

		return MakeUnique<ASTPointerValue>(m_Compiler, SourceLocation(tempLineCount), expression, level);
	}
	else if (m_CurrToken == HazeToken::Assign)
	{
		GetNextToken();
		auto assignExpression = ParseExpression();
		return MakeUnique<ASTPointerValue>(m_Compiler, SourceLocation(tempLineCount), expression, level, std::move(assignExpression));
	}
	else
	{
		return MakeUnique<ASTPointerValue>(m_Compiler, SourceLocation(tempLineCount), expression, level);
	}
}

Unique<ASTBase> Parse::ParseNeg()
{
	uint32 tempLineCount = m_LineCount;
	bool isNumberNeg = m_CurrToken == HazeToken::Sub;

	GetNextToken();
	auto expression = ParseExpression();

	return MakeUnique<ASTNeg>(m_Compiler, SourceLocation(tempLineCount), expression, isNumberNeg);
}

Unique<ASTBase> Parse::ParseNullPtr()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	return MakeUnique<ASTNullPtr>(m_Compiler, SourceLocation(tempLineCount));
}

Unique<ASTBase> Parse::ParseGetAddress()
{
	uint32 tempLineCount = m_LineCount;
	GetNextToken();
	auto expression = ParseExpression(s_HashMap_OperatorPriority.find(HazeToken::GetAddress)->second);
	m_NeedParseNextStatement = tempLineCount != m_LineCount;

	return MakeUnique<ASTGetAddress>(m_Compiler, SourceLocation(tempLineCount), expression);
}

Unique<ASTBase> Parse::ParseLeftBrace()
{
	uint32 tempLineCount = m_LineCount;
	V_Array<Unique<ASTBase>> elements;

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

	return MakeUnique<ASTInitializeList>(m_Compiler, SourceLocation(tempLineCount), elements);
}

Unique<ASTBase> Parse::ParseNot()
{
	int tempLineCount = m_LineCount;

	GetNextToken();
	auto expression = ParseExpression(7000);

	return MakeUnique<ASTNot>(m_Compiler, SourceLocation(tempLineCount), expression);
}

//Unique<ASTBase> Parse::ParseOperatorAssign()
//{
//	HazeToken Token = m_CurrToken;
//
//	auto Value = ParseExpression();
//
//	GetNextToken();
//	return MakeUnique<ASTOperetorAssign>(Compiler, SourceLocation(m_LineCount) Token, Value);
//}

Unique<ASTBase> Parse::ParseMultiExpression()
{
	V_Array<Unique<ASTBase>> expressions;

	GetNextToken();
	while (auto e = ParseExpression())
	{
		expressions.push_back(std::move(e));

		if (TokenIs(HazeToken::RightBrace))
		{
			break;
		}
	}

	return MakeUnique<ASTMultiExpression>(m_Compiler, SourceLocation(m_LineCount), m_StackSectionSignal.top(), expressions);
}

Unique<ASTFunctionSection> Parse::ParseFunctionSection()
{
	if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("函数群期望 {")))
	{
		V_Array<Unique<ASTFunction>> functions;

		GetNextToken();
		while (m_CurrToken != HazeToken::RightBrace)
		{
			functions.push_back(ParseFunction());
		}

		GetNextToken();
		return MakeUnique<ASTFunctionSection>(m_Compiler,/* SourceLocation(m_LineCount),*/ functions);
	}

	return nullptr;
}

Unique<ASTFunction> Parse::ParseFunction(const HString* className)
{
	m_StackSectionSignal.push(HazeSectionSignal::Local);
	uint32 tempLineCount = m_LineCount;

	//获得函数返回类型及是自定义类型时获得类型名字
	HazeDefineType funcType;
	funcType.PrimaryType = GetValueTypeByToken(m_CurrToken);
	GetValueType(funcType);
	
	uint32 startLineCount = m_LineCount;
	HString functionName;

	//获得函数名
	if (ExpectNextTokenIs(HazeToken::Identifier))
	{
		functionName = m_CurrLexeme;
		if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("函数参数定义需要 (")))
		{
			V_Array<Unique<ASTBase>> params;

			if (className && !className->empty())
			{
				HazeDefineVariable thisParam;
				thisParam.Name = HAZE_CLASS_THIS;
				thisParam.Type.PrimaryType = HazeValueType::PointerClass;
				thisParam.Type.CustomName = className->c_str();

				params.push_back(MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount), HazeSectionSignal::Local, thisParam, nullptr));
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

			if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("函数体需要 {")))
			{
				startLineCount = m_LineCount;
				Unique<ASTBase> body = ParseMultiExpression();

				if (TokenIs(HazeToken::RightBrace, H_TEXT("函数体需要 }")))
				{
					m_StackSectionSignal.pop();
					tempLineCount = m_LineCount;

					GetNextToken();
					return MakeUnique<ASTFunction>(m_Compiler, SourceLocation(startLineCount), SourceLocation(tempLineCount),
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
			V_Array<Unique<ASTBase>> params;

			if (className && !className->empty())
			{
				HazeDefineVariable thisParam;
				thisParam.Name = HAZE_CLASS_THIS;
				thisParam.Type.PrimaryType = HazeValueType::PointerClass;
				thisParam.Type.CustomName = className->c_str();

				params.push_back(MakeUnique<ASTVariableDefine>(m_Compiler, SourceLocation(tempLineCount),
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

			if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("类的构造函数体需要 {")))
			{
				startLineCount = m_LineCount;
				Unique<ASTBase> body = ParseMultiExpression();

				if (TokenIs(HazeToken::RightBrace, H_TEXT("类的构造函数体需要 }")))
				{
					m_StackSectionSignal.pop();
					tempLineCount = m_LineCount;

					GetNextToken();
					return MakeUnique<ASTFunction>(m_Compiler, SourceLocation(startLineCount), SourceLocation(tempLineCount),
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

Unique<ASTLibrary> Parse::ParseLibrary()
{
	HazeLibraryType libType = GetHazeLibraryTypeByToken(m_CurrToken);
	GetNextToken();
	HString standardLibraryName = m_CurrLexeme;

	m_StackSectionSignal.push(HazeSectionSignal::Global);

	if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("标准库需要 {")))
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

		return MakeUnique<ASTLibrary>(m_Compiler, /*SourceLocation(m_LineCount),*/ standardLibraryName, libType, functionDefines, classDefines);
	}

	return nullptr;
}

Unique<ASTClassDefine> Parse::ParseLibrary_ClassDefine()
{
	m_CurrParseClass = m_CurrLexeme;
	m_CurrParseClass.clear();
	/*V_Array<V_Array<Unique<ASTBase>>> Data;
	V_Array<Unique<ASTFunctionDefine>> Function;
	return MakeUnique<ASTClassDefine>(Compiler, SourceLocation(m_LineCount) CurrLexeme, Data, Function);*/
	return nullptr;
}

V_Array<Unique<ASTFunctionDefine>> Parse::ParseLibrary_FunctionDefine()
{
	V_Array<Unique<ASTFunctionDefine>> functionDefines;

	if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("标准库函数群需要 {")))
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
			if (ExpectNextTokenIs(HazeToken::Identifier, H_TEXT("库函数命名错误")))
			{
				HString functionName = m_CurrLexeme;
				if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("函数参数定义需要 (")))
				{
					V_Array<Unique<ASTBase>> params;

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
					functionDefines.push_back(MakeUnique<ASTFunctionDefine>(m_Compiler, /*SourceLocation(m_LineCount),*/ functionName, funcType, params));
					GetNextToken();
				}
			}
		}

		GetNextToken();
		//return MakeUnique<ASTStandardLibrary>(Compiler, SourceLocation(m_LineCount) StandardLibraryName, Vector_FunctionDefine);
	}

	return functionDefines;
}

Unique<ASTBase> Parse::ParseImportModule()
{
	if (ExpectNextTokenIs(HazeToken::Identifier))
	{
		HString name = m_CurrLexeme;
		return MakeUnique<ASTImportModule>(m_Compiler, SourceLocation(m_LineCount), name);
	}
	else
	{
		HAZE_LOG_ERR_W("解析错误: 引入模块<%s>错误! <%s>文件<%d>行!\n", m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
	}

	return nullptr;
}

Unique<ASTClass> Parse::ParseClass()
{
	if (ExpectNextTokenIs(HazeToken::Identifier))
	{
		m_CurrParseClass = m_CurrLexeme;
		HString name = m_CurrLexeme;
		V_Array<HString> parentClasses;

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
						HAZE_LOG_ERR_W("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n", name.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
					}
				}
			}
			else
			{
				HAZE_LOG_ERR_W("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n", name.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
			}
		}

		if (TokenIs(HazeToken::LeftBrace))
		{
			m_StackSectionSignal.push(HazeSectionSignal::Class);

			V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> classDatas;
			Unique<ASTClassFunctionSection> classFunctions;

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
						HAZE_LOG_ERR_W("解析错误: 类中相同的区域<%s>只能存在一种! <%s>文件<%d>行!", name.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
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
			return MakeUnique<ASTClass>(m_Compiler, /*SourceLocation(m_LineCount),*/ name, parentClasses, classDatas, classFunctions);
		}
	}
	else
	{
		HAZE_LOG_ERR_W("解析错误: 类名<%s>错误! <%s>文件<%d>行!", m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
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

		while (m_CurrToken == HazeToken::ClassPublic || m_CurrToken == HazeToken::ClassPrivate || m_CurrToken == HazeToken::ClassProtected)
		{
			if (m_CurrToken == HazeToken::ClassPublic)
			{
				if (!HasData(classDatas, HazeDataDesc::ClassMember_Local_Public))
				{
					classDatas.push_back(std::move(
						Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Public,
						std::move(V_Array<Unique<ASTBase>>{}) })));

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
					HAZE_LOG_ERR_W("解析错误: 类的公有区域只能定义一次! <%s>文件<%d>行!\n", m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
			else if (m_CurrToken == HazeToken::ClassPrivate)
			{
				if (!HasData(classDatas, HazeDataDesc::ClassMember_Local_Private))
				{
					classDatas.push_back(std::move(
						Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Private,
						std::move(V_Array<Unique<ASTBase>>{}) })));

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
					HAZE_LOG_ERR_W("解析错误: 类的私有区域只能定义一次! <%s>文件<%d>行!\n", m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
			else if (m_CurrToken == HazeToken::ClassProtected)
			{
				if (!HasData(classDatas, HazeDataDesc::ClassMember_Local_Protected))
				{
					classDatas.push_back(std::move(
						Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>({ HazeDataDesc::ClassMember_Local_Protected,
						std::move(V_Array<Unique<ASTBase>>{}) })));

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
					HAZE_LOG_ERR_W("解析错误: 类的保护区域只能定义一次! <%s>文件<%d>行!\n", m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
		}

		if (m_CurrToken != HazeToken::RightBrace)
		{
			HAZE_LOG_ERR_W("解析错误: 类需要 }! <%s>文件<%d>行!\n", m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
		}

		GetNextToken();
	}

	return classDatas;
}

Unique<ASTClassFunctionSection> Parse::ParseClassFunction(const HString& className)
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

		while (m_CurrToken == HazeToken::ClassPublic || m_CurrToken == HazeToken::ClassPrivate || m_CurrToken == HazeToken::ClassProtected)
		{
			if (m_CurrToken == HazeToken::ClassPublic)
			{
				if (!(HasData(classFunctions, HazeDataDesc::ClassFunction_Local_Public)))
				{
					classFunctions.push_back(std::move(
						Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Public,
						std::move(V_Array<Unique<ASTFunction>>{}) })));

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
						Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Private,
						std::move(V_Array<Unique<ASTFunction>>{}) })));

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
						Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>({ HazeDataDesc::ClassFunction_Local_Protected,
						std::move(V_Array<Unique<ASTFunction>>{}) })));

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
		return MakeUnique<ASTClassFunctionSection>(m_Compiler, /*SourceLocation(m_LineCount),*/ classFunctions);
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseArrayLength()
{
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("获得数组长度需要 （")))
	{
		GetNextToken();
		auto variable = ParseExpression();

		if (TokenIs(HazeToken::RightParentheses, H_TEXT("获得数组长度需要 )")))
		{
			GetNextToken();
			return MakeUnique<ASTArrayLength>(m_Compiler, m_LineCount, variable);
		}
	}

	return nullptr;
}

Unique<ASTBase> Parse::ParseSizeOf()
{
	if (ExpectNextTokenIs(HazeToken::LeftParentheses, H_TEXT("获得字节大小需要 （")))
	{
		GetNextToken();

		m_DefineVariable.Name.clear();
		m_DefineVariable.Type.Reset();

		Unique<ASTBase> variable = nullptr;
		if (TokenIs(HazeToken::Identifier))
		{
			variable = ParseExpression();
		}
		else
		{
			m_DefineVariable.Type.PrimaryType = GetValueTypeByToken(m_CurrToken);
			GetValueType(m_DefineVariable.Type);
		}

		if (ExpectNextTokenIs(HazeToken::RightParentheses, H_TEXT("获得字节大小需要 )")))
		{
			GetNextToken();
			return MakeUnique<ASTSizeOf>(m_Compiler, m_LineCount, m_DefineVariable.Type, variable);
		}
	}

	return nullptr;
}

Unique<ASTEnum> Parse::ParseEnum()
{
	m_StackSectionSignal.push(HazeSectionSignal::Enum);
	if (ExpectNextTokenIs(HazeToken::Identifier, H_TEXT("枚举名错误")))
	{
		int line = m_LineCount;
		HString name = m_CurrLexeme;

		if (ExpectNextTokenIs(HazeToken::Colon, H_TEXT("枚举定义需要 : 符号")))
		{
			HazeValueType type = GetValueTypeByToken(GetNextToken());
			if (IsIntegerType(type))
			{
				if (ExpectNextTokenIs(HazeToken::LeftBrace, H_TEXT("枚举定义缺少 { 符号")))
				{
					V_Array<Pair<HString, Unique<ASTBase>>> enums;

					GetNextToken();
					while (TokenIs(HazeToken::Identifier))
					{
						enums.push_back({ m_CurrLexeme, nullptr });

						if (ExpectNextTokenIs(HazeToken::Assign))
						{
							GetNextToken();
							enums.back().second = ParseExpression();
						}
					}

					if (TokenIs(HazeToken::RightBrace, H_TEXT("枚举定义缺少 } 符号")))
					{
						GetNextToken();
						
						m_StackSectionSignal.pop();
						return MakeUnique<ASTEnum>(m_Compiler, line, name, type, enums);
					}
				}
			}
		}
	}

	m_StackSectionSignal.pop();
	return nullptr;
}

void Parse::ParseTemplate()
{
	if (ExpectNextTokenIs(HazeToken::Less))
	{
		V_Array<HString> templateTypes;
		do
		{
			if (ExpectNextTokenIs(HazeToken::TypeName))
			{
				if (ExpectNextTokenIs(HazeToken::Identifier, H_TEXT("模板类型名定义错误")))
				{
					templateTypes.push_back(m_CurrLexeme);
				}
			}
		} while (TokenIs(HazeToken::Comma));

		if (ExpectNextTokenIs(HazeToken::Greater))
		{
			if (ExpectNextTokenIs(HazeToken::Class))
			{
				if (ExpectNextTokenIs(HazeToken::Identifier, H_TEXT("模板类名定义错误")))
				{
					HString templateClassName = m_CurrLexeme;
					const HChar* start = m_CurrCode;
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
									HAZE_LOG_ERR_W("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n", templateClassName.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
								}

								line = m_LineCount;
							}
						}
						else
						{
							HAZE_LOG_ERR_W("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n", templateClassName.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
						}
					}
					
					if (TokenIs(HazeToken::LeftBrace))
					{
						V_Array<uint32> stack(1);
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

						HString templateText(start, m_CurrCode);
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

Unique<ASTTemplateBase> Parse::ParseTemplateClass(V_Array<HString>& templateTypes)
{
	m_CurrParseClass = m_CurrLexeme;
	HString name = m_CurrLexeme;
	V_Array<HString> parentClasses;

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
					HAZE_LOG_ERR_W("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n", name.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
				}
			}
		}
		else
		{
			HAZE_LOG_ERR_W("解析错误: 类<%s>继承<%s>错误! <%s>文件<%d>行!\n", name.c_str(), m_CurrLexeme.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
		}
	}

	if (TokenIs(HazeToken::LeftBrace))
	{
		m_StackSectionSignal.push(HazeSectionSignal::Class);

		V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> classDatas;
		Unique<ASTClassFunctionSection> classFunctions;

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
					HAZE_LOG_ERR_W("解析错误: 类中相同的区域<%s>只能存在一种! <%s>文件<%d>行!", name.c_str(), m_Compiler->GetCurrModuleName().c_str(), m_LineCount);
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
		return nullptr; //return MakeUnique<ASTTemplateClass>(m_Compiler, /*SourceLocation(m_LineCount),*/ name, parentClasses, classDatas, classFunctions);
	}
}
Unique<ASTTemplateBase> Parse::ParseTemplateFunction(V_Array<HString>& templateTypes)
{
	return Unique<ASTTemplateBase>();
}

bool Parse::ExpectNextTokenIs(HazeToken token, const HChar* errorInfo)
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

bool Parse::TokenIs(HazeToken token, const HChar* errorInfo)
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

bool Parse::IsHazeSignalToken(const HChar* hChar, const HChar*& outChar, uint32 charSize)
{
	static std::unordered_set<HString> s_HashSet_TokenText =
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
		TOKEN_QUESTIOB_COLON, TOKEN_TWO_COLON,
	};

	static HString s_WS;

	s_WS.resize(charSize);
	memcpy(s_WS.data(), hChar, sizeof(HChar) * charSize);

	auto iter = s_HashSet_TokenText.find(s_WS);
	if (iter != s_HashSet_TokenText.end())
	{
		outChar = iter->c_str();
	}

	return iter != s_HashSet_TokenText.end();
}

bool Parse::IsPointerOrRef(const HString& str, HazeToken& outToken)
{
	HString typeName = str.substr(0, str.length() - 1);
	HString pointerChar = str.substr(str.length() - 1, 1);

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
				PARSE_ERR_W("最多3层指针 !\n");
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

void Parse::GetTemplateRealValueType(const HString& str, HazeDefineType& inType)
{
	if (m_IsParseTemplate)
	{
		HString pointerChar = str.substr(str.length() - 1, 1);

		if (pointerChar == MUL_STR)
		{
			int pointerLevel = 1;

			HString typeName = str.substr(0, str.length() - 1);
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

void Parse::IncLineCount(bool insert)
{
	m_LineCount++;

#if HAZE_DEBUG_ENABLE
	HChar code[20];
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