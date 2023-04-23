#pragma once

#include <stack>
#include <string>

#include "Haze.h"

class OpCodeGenerator;
class ASTBase;

class ASTVariableDefine;

class ASTClass;
class ASTClassDefine;

class ASTFunction;
class ASTFunctionSection;
class ASTFunctionDefine;
class ASTClassFunctionSection;

class ASTStandardLibrary;

class Parse
{
public:
	Parse(class HazeVM* VM);
	~Parse();

	void InitializeFile(const HAZE_STRING& FilePath);

	void InitializeString(const HAZE_STRING& String);

	void ParseContent();

	HazeToken GetNextToken();

	const HAZE_STRING& GetCurrLexeme() const { return CurrLexeme; }

	const HAZE_STRING& GetLookAtAheadChar() {}

	static bool TokenIsNone(HazeToken Token) { return Token == HazeToken::None; }

private:
	std::unique_ptr<ASTBase> HandleParseExpression();

	std::unique_ptr<ASTBase> ParseExpression();

	std::unique_ptr<ASTBase> ParseUnaryExpression();

	std::unique_ptr<ASTBase> ParseBinaryOperateExpression(int Prec, std::unique_ptr<ASTBase> Left);

	std::unique_ptr<ASTBase> ParsePrimary();

	std::unique_ptr<ASTBase> ParseIdentifer();

	std::unique_ptr<ASTVariableDefine> ParseVariableDefine();

	std::unique_ptr<ASTBase> ParseStringText();

	std::unique_ptr<ASTBase> ParseBoolExpression();

	std::unique_ptr<ASTBase> ParseNumberExpression();

	std::unique_ptr<ASTBase> ParseIfExpression(bool Recursion = false);

	std::unique_ptr<ASTBase> ParseForExpression();

	std::unique_ptr<ASTBase> ParseWhileExpression();

	std::unique_ptr<ASTBase> ParseReturn();

	std::unique_ptr<ASTBase> ParseNew();

	std::unique_ptr<ASTBase> ParseLeftBrackets();

	std::unique_ptr<ASTBase> ParseInc();

	std::unique_ptr<ASTBase> ParseDec();

	std::unique_ptr<ASTBase> ParseLeftParentheses();

	std::unique_ptr<ASTBase> ParseLeftBrace();

	std::unique_ptr<ASTBase> ParseOperatorAssign();

	std::unique_ptr<ASTBase> ParseMultiExpression();

	std::unique_ptr<ASTFunctionSection> ParseFunctionSection();

	std::unique_ptr<ASTFunction> ParseMainFunction();

	std::unique_ptr<ASTStandardLibrary> ParseStandardLibrary();

	std::unique_ptr<ASTClassDefine> ParseStandardLibrary_ClassDefine();

	std::vector<std::unique_ptr<ASTFunctionDefine>> ParseStandardLibrary_FunctionDefine();

	std::unique_ptr<ASTBase> ParseImportModule();

	std::unique_ptr<ASTClass> ParseClass();

	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTVariableDefine>>>> ParseClassData();

	std::unique_ptr<ASTClassFunctionSection> ParseClassFunction(const HAZE_STRING& ClassName);

private:
	std::unique_ptr<ASTFunction> ParseFunction(const HAZE_STRING* ClassName = nullptr);

	bool ExpectNextTokenIs(HazeToken Token, const HAZE_CHAR* ErrorInfo = nullptr);

	bool TokenIs(HazeToken Token, const HAZE_CHAR* ErrorInfo = nullptr);

	bool IsHazeSignalToken(const HAZE_CHAR* Char, const HAZE_CHAR*& OutChar, uint32 CharSize = 1);

	bool IsPointer(const HAZE_STRING& Str, HazeToken& OutToken);

private:
	class HazeVM* VM;

	HAZE_STRING CodeText;

	HazeToken CurrToken;
	const HAZE_CHAR* CurrCode;
	HAZE_STRING CurrLexeme;
	HAZE_STRING CurrPreLexeme;

	std::stack<HazeSectionSignal> StackSectionSignal;

	std::shared_ptr<OpCodeGenerator> Generator;

	HazeDefineVariable DefineVariable;
	HAZE_STRING CurrParseClass;
};
