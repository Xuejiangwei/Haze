#pragma once

class HazeCompiler;

class ASTBase;
class ASTVariableDefine;
class ASTClass;
class ASTEnum;
class ASTClassDefine;
class ASTFunction;
class ASTFunctionSection;
class ASTFunctionDefine;
class ASTClassFunctionSection;
class ASTLibrary;
class ASTTemplateBase;

class Parse
{
	friend struct TempCurrCode;
public:
	Parse(HazeCompiler* compiler);

	~Parse();

	void InitializeFile(const HString& filePath);

	void InitializeString(const HString& str, uint32 startLine = 0);

	void ParseContent();

	void ParseTemplateContent(const HString& moduleName, const HString& templateName, const V_Array<HString>& templateTypes,
		const V_Array<HazeDefineType>& templateRealTypes);

	HazeToken GetNextToken(bool clearLexeme = true);

	const HString& GetCurrLexeme() const { return m_CurrLexeme; }

	static bool TokenIsNone(HazeToken token) { return token == HazeToken::None; }

private:
	Unique<ASTBase> HandleParseExpression();

	Unique<ASTBase> ParseExpression(int prec = 0);

	Unique<ASTBase> ParseUnaryExpression();

	Unique<ASTBase> ParseBinaryOperateExpression(int prec, Unique<ASTBase> left);

	Unique<ASTBase> ParsePrimary();

	Unique<ASTBase> ParseIdentifer();

	Unique<ASTBase> ParseVariableDefine();
	Unique<ASTBase> ParseVariableDefine_MultiVariable();
	Unique<ASTBase> ParseVariableDefine_Array(TemplateDefineTypes& templateTypes);
	Unique<ASTBase> ParseVariableDefine_Class(TemplateDefineTypes& templateTypes);
	Unique<ASTBase> ParseVariableDefine_Function(TemplateDefineTypes& templateTypes);

	Unique<ASTBase> ParseStringText();

	Unique<ASTBase> ParseBoolExpression();

	Unique<ASTBase> ParseNumberExpression();

	Unique<ASTBase> ParseIfExpression(bool recursion = false);

	Unique<ASTBase> ParseForExpression();

	Unique<ASTBase> ParseWhileExpression();

	Unique<ASTBase> ParseBreakExpression();

	Unique<ASTBase> ParseContinueExpression();

	Unique<ASTBase> ParseReturn();

	Unique<ASTBase> ParseNew();

	Unique<ASTBase> ParseLeftBrace();

	Unique<ASTBase> ParseNot();

	Unique<ASTBase> ParseLeftParentheses();

	//Unique<ASTBase> ParsePointerValue();

	Unique<ASTBase> ParseNeg();

	Unique<ASTBase> ParseNullPtr();

	Unique<ASTBase> ParseGetAddress();

	Unique<ASTBase> ParseInc();

	Unique<ASTBase> ParseDec();

	Unique<ASTBase> ParseThreeOperator(Unique<ASTBase> condition);

	Unique<ASTBase> ParseMultiExpression();

	Unique<ASTFunctionSection> ParseFunctionSection();

	Unique<ASTLibrary> ParseLibrary();

	V_Array<Unique<ASTFunctionDefine>> ParseLibrary_FunctionDefine();

	Unique<ASTBase> ParseImportModule();

	Unique<ASTClass> ParseClass();

	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> ParseClassData();

	Unique<ASTClassFunctionSection> ParseClassFunction(const HString& className);

	Unique<ASTBase> ParseSizeOf();

	Unique<ASTEnum> ParseEnum();

	void ParseTemplate();

	Unique<ASTTemplateBase> ParseTemplateClass(V_Array<HString>& templateTypes);

	Unique<ASTTemplateBase> ParseTemplateFunction(V_Array<HString>& templateTypes);

private:
	Unique<ASTFunction> ParseFunction(const HString* className = nullptr);

	bool ExpectNextTokenIs(HazeToken token, const HChar* errorInfo = nullptr);

	bool NextTokenNotIs(HazeToken token) { return GetNextToken() != token; }

	bool TokenIs(HazeToken token, const HChar* errorInfo = nullptr);

	bool IsHazeSignalToken(const HChar* hChar, const HChar*& outChar, uint32 charSize = 1);

	bool IsNumberType(const HString& str, HazeToken& outToken);

	void GetValueType(HazeDefineType& inType);

	void GetTemplateRealValueType(const HString& str, HazeDefineType& inType);

	void ParseTemplateTypes(TemplateDefineTypes& templateTypes);

	//void ParseVariableType();

	void IncLineCount(bool insert = false);

private:
	HazeCompiler* m_Compiler;

	HazeLibraryType m_LibraryType;
	HazeToken m_CurrToken;
	const HChar* m_CurrCode;
	HString m_CodeText;
	HString m_CurrLexeme;
	Pair<HString, int> m_CurrPreLexeme;		//LexemeString, skip char count

	std::stack<HazeSectionSignal> m_StackSectionSignal;

	HazeDefineVariable m_DefineVariable;
	HString m_CurrParseClass;

	int m_LeftParenthesesExpressionCount;
	uint32 m_LineCount;
	//bool m_NeedParseNextStatement;

	bool m_IsParseArray;
	bool m_IsParseTemplate;
	const V_Array<HString>* m_TemplateTypes;
	const V_Array<HazeDefineType>* m_TemplateRealTypes;
};
