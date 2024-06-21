#pragma once

#include <unordered_map>
#include <fstream>

#include "HazeHeader.h"

class HazeCompiler;
class HazeCompilerValue;
class HazeBaseBlock;
class HazeCompilerFunction;
class HazeCompilerClass;
class HazeCompilerEnum;
class HazeCompilerTemplateFunction;
class HazeCompilerTemplateClass;

struct TemplateCacheTextData
{
	uint32 StartLine;
	HAZE_STRING Text;
	std::vector<HAZE_STRING> Types;
};

class HazeCompilerModule
{
	friend class HazeCompiler;
	friend struct PushTempRegister;
public:

	HazeCompilerModule(HazeCompiler* compiler, const HAZE_STRING& moduleName);

	~HazeCompilerModule();

	HazeCompiler* GetCompiler() { return m_Compiler; }

	const HAZE_STRING& GetName() const;

	const HAZE_STRING& GetCurrClassName() const { return m_CurrClass; }

	HazeLibraryType GetModuleLibraryType() { return m_ModuleLibraryType; }

	void MarkLibraryType(HazeLibraryType type);

	void RestartTemplateModule(const HAZE_STRING& moduleName);

	void FinishModule();

	std::shared_ptr<HazeCompilerClass> CreateClass(const HAZE_STRING& name, std::vector<HazeCompilerClass*>& parentClass,
		std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& classData);

	std::shared_ptr<HazeCompilerEnum> CreateEnum(const HAZE_STRING& name, HazeValueType parentType,
		std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& enumValues);

	void FinishCreateClass();

	std::shared_ptr<HazeCompilerClass> GetClass(const HAZE_STRING& className);

	uint32 GetClassSize(const HAZE_STRING& className);

	std::shared_ptr<HazeCompilerFunction> GetCurrFunction();

	std::shared_ptr<HazeCompilerFunction> CreateFunction(const HAZE_STRING& name, HazeDefineType& type, std::vector<HazeDefineVariable>& params);

	std::shared_ptr<HazeCompilerFunction> CreateFunction(std::shared_ptr<HazeCompilerClass> compilerClass, const HAZE_STRING& name, HazeDefineType& type, std::vector<HazeDefineVariable>& params);

	void BeginCreateFunctionParamVariable() { m_IsBeginCreateFunctionVariable = true; }

	void EndCreateFunctionParamVariable() { m_IsBeginCreateFunctionVariable = false; }

	bool IsBeginCreateFunctionVariable() const { return m_IsBeginCreateFunctionVariable; }

	void FinishFunction();

	std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> GetFunction(const HAZE_STRING& name);

	void StartCacheTemplate(HAZE_STRING& templateName, uint32 startLine, HAZE_STRING& templateText, std::vector<HAZE_STRING>& templateTypes);

	bool IsTemplateClass(const HAZE_STRING& name);

	bool ResetTemplateClassRealName(HAZE_STRING& inName, const std::vector<HazeDefineType>& templateTypes);

	std::shared_ptr<HazeCompilerEnum> GetEnum(const HAZE_STRING& name);

	std::shared_ptr<HazeCompilerValue> GetOrCreateGlobalStringVariable(const HAZE_STRING& str);

	uint32 GetGlobalStringIndex(std::shared_ptr<HazeCompilerValue> value);

	void PreStartCreateGlobalVariable();

	void EndCreateGlobalVariable();

	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(const HazeDefineVariable& var, std::shared_ptr<HazeCompilerValue> refValue = nullptr,
		std::vector<std::shared_ptr<HazeCompilerValue>> arraySize = {}, std::vector<HazeDefineType>* params = nullptr);

	std::shared_ptr<HazeCompilerValue> GetGlobalVariable(const HAZE_STRING& name);

	bool GetGlobalVariableName(const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName);

	bool GetGlobalVariableName(const HazeCompilerValue* value, HAZE_STRING& outName);

	static void GenVariableHzic(HazeCompilerModule* compilerModule, HAZE_STRING_STREAM& hss, const std::shared_ptr<HazeCompilerValue>& value, int index = -1);

private:
	std::shared_ptr<HazeCompilerValue> CreateAdd(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateSub(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateMul(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateDiv(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateMod(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitAnd(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitOr(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitNeg(std::shared_ptr<HazeCompilerValue> value);

	std::shared_ptr<HazeCompilerValue> CreateBitXor(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateShl(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateShr(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, bool isAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateNot(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right);

	std::shared_ptr<HazeCompilerValue> CreateNeg(std::shared_ptr<HazeCompilerValue> value);

	std::shared_ptr<HazeCompilerValue> CreateInc(std::shared_ptr<HazeCompilerValue> value, bool isPreInc);

	std::shared_ptr<HazeCompilerValue> CreateDec(std::shared_ptr<HazeCompilerValue> value, bool isPreDec);

	std::shared_ptr<HazeCompilerValue> CreateArrayInit(std::shared_ptr<HazeCompilerValue> array, std::shared_ptr<HazeCompilerValue> initList);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> callFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& params, std::shared_ptr<HazeCompilerValue> thisPointerTo = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerValue> pointerFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& params, std::shared_ptr<HazeCompilerValue> thisPointerTo = nullptr);

	std::shared_ptr<HazeCompilerValue> GenIRCode_BinaryOperater(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right, InstructionOpCode opCode);

	void GenIRCode_UnaryOperator(std::shared_ptr<HazeCompilerValue> value, InstructionOpCode opCode);

	void GenIRCode_Ret(std::shared_ptr<HazeCompilerValue> value);

	void GenIRCode_Cmp(HazeCmpType cmpType, std::shared_ptr<HazeBaseBlock> ifJmpBlock, std::shared_ptr<HazeBaseBlock> elseJmpBlock);

	void GenIRCode_JmpTo(std::shared_ptr<HazeBaseBlock> block);

	//static void GenValueHzicText(HazeCompilerModule* Module, HAZE_STRING_STREAM& HSS, const std::shared_ptr<HazeCompilerValue>& Value, int Index = -1);

private:
	void FunctionCall(HAZE_STRING_STREAM& hss, const HAZE_STRING& callName, uint32& size, std::vector<std::shared_ptr<HazeCompilerValue>>& params, std::shared_ptr<HazeCompilerValue> thisPointerTo);

	void GenCodeFile();

	void GenICode();

private:
	HazeCompiler* m_Compiler;

	HazeLibraryType m_ModuleLibraryType;

	HAZE_OFSTREAM m_FS_I_Code;

	HAZE_STRING m_CurrClass;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerClass>> m_HashMap_Classes;

	HAZE_STRING m_CurrFunction;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerFunction>> m_HashMap_Functions;

	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerEnum>> m_HashMap_Enums;

	//之后考虑全局变量放在一个block里面，复用其中的函数与成员变量
	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> m_Variables; //全局变量
	std::vector<std::pair<int, int>> m_VariablesAddress;
	std::vector<HAZE_STRING> m_ModuleIRCodes;

	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerValue>> m_HashMap_StringTable;
	std::unordered_map<int, const HAZE_STRING*> m_HashMap_StringMapping;

	std::unordered_map<HAZE_STRING, TemplateCacheTextData> m_HashMap_TemplateText;

	std::vector<HazeCompilerModule*> m_ImportModules;

	bool m_IsBeginCreateFunctionVariable;
	bool m_IsGenTemplateCode;
};
