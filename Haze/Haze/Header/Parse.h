#pragma once

#include <stack>
#include <string>

#include "HazeHeader.h"

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
class ASTTemplateBase;

class Parse
{
public:
	Parse(HazeCompiler* compiler);

	~Parse();

	void InitializeFile(const HAZE_STRING& filePath);

	void InitializeString(const HAZE_STRING& str);

	void ParseContent();

	void ParseTemplateContent(const HAZE_STRING& moduleName, const HAZE_STRING& templateName, const std::vector<HAZE_STRING>& templateTypes, const std::vector<HazeDefineType>& templateRealTypes);

	HazeToken GetNextToken();

	const HAZE_STRING& GetCurrLexeme() const { return m_CurrLexeme; }

	static bool TokenIsNone(HazeToken token) { return token == HazeToken::None; }

private:
	std::unique_ptr<ASTBase> HandleParseExpression();

	std::unique_ptr<ASTBase> ParseExpression(int prec = 0);

	std::unique_ptr<ASTBase> ParseUnaryExpression();

	std::unique_ptr<ASTBase> ParseBinaryOperateExpression(int prec, std::unique_ptr<ASTBase> left);

	std::unique_ptr<ASTBase> ParsePrimary();

	std::unique_ptr<ASTBase> ParseIdentifer();

	std::unique_ptr<ASTBase> ParseVariableDefine();

	std::unique_ptr<ASTBase> ParseStringText();

	std::unique_ptr<ASTBase> ParseBoolExpression();

	std::unique_ptr<ASTBase> ParseNumberExpression();

	std::unique_ptr<ASTBase> ParseIfExpression(bool recursion = false);

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

	std::unique_ptr<ASTBase> ParseThreeOperator(std::unique_ptr<ASTBase> condition);

	std::unique_ptr<ASTBase> ParseMultiExpression();

	std::unique_ptr<ASTFunctionSection> ParseFunctionSection();

	std::unique_ptr<ASTFunction> ParseMainFunction();

	std::unique_ptr<ASTLibrary> ParseLibrary();

	std::unique_ptr<ASTClassDefine> ParseLibrary_ClassDefine();

	std::vector<std::unique_ptr<ASTFunctionDefine>> ParseLibrary_FunctionDefine();

	std::unique_ptr<ASTBase> ParseImportModule();

	std::unique_ptr<ASTClass> ParseClass();

	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> ParseClassData();

	std::unique_ptr<ASTClassFunctionSection> ParseClassFunction(const HAZE_STRING& className);

	std::unique_ptr<ASTBase> ParseEnum();

	void ParseTemplate();

	std::unique_ptr<ASTTemplateBase> ParseTemplateClass(std::vector<HAZE_STRING>& templateTypes);

	std::unique_ptr<ASTTemplateBase> ParseTemplateFunction(std::vector<HAZE_STRING>& templateTypes);

private:
	std::unique_ptr<ASTFunction> ParseFunction(const HAZE_STRING* className = nullptr);

	bool ExpectNextTokenIs(HazeToken token, const HAZE_CHAR* errorInfo = nullptr);

	bool TokenIs(HazeToken token, const HAZE_CHAR* errorInfo = nullptr);

	bool IsHazeSignalToken(const HAZE_CHAR* hChar, const HAZE_CHAR*& outChar, uint32 charSize = 1);

	bool IsPointerOrRef(const HAZE_STRING& str, HazeToken& outToken);

	const HazeDefineType* GetTemplateRealValueType(const HAZE_STRING& str, bool isPointer = false);

	HazeToken GetTokenByTemplateType(const HAZE_STRING& str);

	void BackToPreLexemeAndNext();

	void IncLineCount(bool insert = false);

private:
	HazeCompiler* m_Compiler;

	HazeToken m_CurrToken;
	const HAZE_CHAR* m_CurrCode;
	HAZE_STRING m_CodeText;
	HAZE_STRING m_CurrLexeme;
	std::pair<HAZE_STRING, int> m_CurrPreLexeme;		//LexemeString, skip char count

	std::stack<HazeSectionSignal> m_StackSectionSignal;

	HazeDefineVariable m_DefineVariable;
	HAZE_STRING m_CurrParseClass;

	int m_LeftParenthesesExpressionCount;
	uint32 m_LineCount;
	bool m_NeedParseNextStatement;

	bool m_IsParseTemplate;
	const std::vector<HAZE_STRING>* m_TemplateTypes;
	const std::vector<HazeDefineType>* m_TemplateRealTypes;
};
