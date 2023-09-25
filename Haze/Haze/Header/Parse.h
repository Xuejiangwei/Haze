#pragma once

#include <stack>
#include <string>

#include "Haze.h"

class HazeCompiler;

class ASTBase;

class ASTVariableDefine;

class ASTClass;
class ASTClassDefine;

class ASTFunction;
class ASTFunctionSection;
class ASTFunctionDefine;
class ASTClassFunctionSection;

class ASTLibrary;

class Parse
{
public:
	Parse(HazeCompiler* m_Compiler);
	~Parse();

	void InitializeFile(const HAZE_STRING& FilePath);

	void InitializeString(const HAZE_STRING& String);

	void ParseContent();

	HazeToken GetNextToken();

	const HAZE_STRING& GetCurrLexeme() const { return CurrLexeme; }

	const HAZE_STRING& GetLookAtAheadChar() {}

	static bool TokenIsNone(HazeToken m_Token) { return m_Token == HazeToken::None; }

private:
	std::unique_ptr<ASTBase> HandleParseExpression();

	std::unique_ptr<ASTBase> ParseExpression(int Prec = 0);

	std::unique_ptr<ASTBase> ParseUnaryExpression();

	std::unique_ptr<ASTBase> ParseBinaryOperateExpression(int Prec, std::unique_ptr<ASTBase> Left);

	std::unique_ptr<ASTBase> ParsePrimary();

	std::unique_ptr<ASTBase> ParseIdentifer();

	std::unique_ptr<ASTBase> ParseVariableDefine();

	std::unique_ptr<ASTBase> ParseStringText();

	std::unique_ptr<ASTBase> ParseBoolExpression();

	std::unique_ptr<ASTBase> ParseNumberExpression();

	std::unique_ptr<ASTBase> ParseIfExpression(bool Recursion = false);

	std::unique_ptr<ASTBase> ParseForExpression();

	std::unique_ptr<ASTBase> ParseWhileExpression();

	std::unique_ptr<ASTBase> ParseBreakExpression();

	std::unique_ptr<ASTBase> ParseContinueExpression();

	std::unique_ptr<ASTBase> ParseReturn();

	std::unique_ptr<ASTBase> ParseNew();

	std::unique_ptr<ASTBase> ParseLeftBrace();

	std::unique_ptr<ASTBase> ParseNot();

	std::unique_ptr<ASTBase> ParseLeftParentheses();

	std::unique_ptr<ASTBase> ParsePointerValue();

	std::unique_ptr<ASTBase> ParseNeg();

	std::unique_ptr<ASTBase> ParseNullPtr();

	std::unique_ptr<ASTBase> ParseGetAddress();

	std::unique_ptr<ASTBase> ParseInc();

	std::unique_ptr<ASTBase> ParseDec();

	std::unique_ptr<ASTBase> ParseThreeOperator(std::unique_ptr<ASTBase> Condition);

	std::unique_ptr<ASTBase> ParseMultiExpression();

	std::unique_ptr<ASTFunctionSection> ParseFunctionSection();

	std::unique_ptr<ASTFunction> ParseMainFunction();

	std::unique_ptr<ASTLibrary> ParseLibrary();

	std::unique_ptr<ASTClassDefine> ParseLibrary_ClassDefine();

	std::vector<std::unique_ptr<ASTFunctionDefine>> ParseLibrary_FunctionDefine();

	std::unique_ptr<ASTBase> ParseImportModule();

	std::unique_ptr<ASTClass> ParseClass();

	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> ParseClassData();

	std::unique_ptr<ASTClassFunctionSection> ParseClassFunction(const HAZE_STRING& ClassName);

private:
	std::unique_ptr<ASTFunction> ParseFunction(const HAZE_STRING* ClassName = nullptr);

	bool ExpectNextTokenIs(HazeToken m_Token, const HAZE_CHAR* ErrorInfo = nullptr);

	bool TokenIs(HazeToken m_Token, const HAZE_CHAR* ErrorInfo = nullptr);

	bool IsHazeSignalToken(const HAZE_CHAR* Char, const HAZE_CHAR*& OutChar, uint32 CharSize = 1);

	bool IsPointerOrRef(const HAZE_STRING& Str, HazeToken& OutToken);

	void BackToPreLexemeAndNext();

	void IncLineCount(bool Insert = false);

private:
	HazeCompiler* m_Compiler;

	HAZE_STRING CodeText;

	HazeToken CurrToken;
	const HAZE_CHAR* CurrCode;
	HAZE_STRING CurrLexeme;
	std::pair<HAZE_STRING, int> CurrPreLexeme;		//LexemeString, skip char count

	std::stack<HazeSectionSignal> StackSectionSignal;

	HazeDefineVariable m_DefineVariable;
	HAZE_STRING CurrParseClass;

	int LeftParenthesesExpressionCount;

	uint32 LineCount;
	bool NeedParseNextStatement;
};
