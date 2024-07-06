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
	HString Text;
	V_Array<HString> Types;
};

class HazeCompilerModule
{
	friend class HazeCompiler;
	friend struct PushTempRegister;
public:

	HazeCompilerModule(HazeCompiler* compiler, const HString& moduleName, const HString& modulePath);

	~HazeCompilerModule();

	HazeCompiler* GetCompiler() { return m_Compiler; }

	const HString& GetName() const;

	const HString& GetPath() const { return m_Path; }

	const HString& GetCurrClassName() const { return m_CurrClass; }

	HazeLibraryType GetModuleLibraryType() { return m_ModuleLibraryType; }

	void MarkLibraryType(HazeLibraryType type);

	void RestartTemplateModule(const HString& moduleName);

	void FinishModule();

	Share<HazeCompilerClass> CreateClass(const HString& name, V_Array<HazeCompilerClass*>& parentClass,
		V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<HazeCompilerValue>>>>>& classData);

	Share<HazeCompilerEnum> CreateEnum(const HString& name, HazeValueType baseType);

	Share<HazeCompilerEnum> GetCurrEnum();

	void FinishCreateEnum();

	void FinishCreateClass();

	Share<HazeCompilerClass> GetClass(const HString& className);

	uint32 GetClassSize(const HString& className);

	Share<HazeCompilerFunction> GetCurrFunction();

	Share<HazeCompilerFunction> CreateFunction(const HString& name, HazeDefineType& type, V_Array<HazeDefineVariable>& params);

	Share<HazeCompilerFunction> CreateFunction(Share<HazeCompilerClass> compilerClass, const HString& name, HazeDefineType& type, V_Array<HazeDefineVariable>& params);

	void BeginCreateFunctionParamVariable() { m_IsBeginCreateFunctionVariable = true; }

	void EndCreateFunctionParamVariable() { m_IsBeginCreateFunctionVariable = false; }

	bool IsBeginCreateFunctionVariable() const { return m_IsBeginCreateFunctionVariable; }

	void FinishFunction();

	Pair<Share<HazeCompilerFunction>, Share<HazeCompilerValue>> GetFunction(const HString& name);

	void StartCacheTemplate(HString& templateName, uint32 startLine, HString& templateText, V_Array<HString>& templateTypes);

	bool IsTemplateClass(const HString& name);

	bool ResetTemplateClassRealName(HString& inName, const V_Array<HazeDefineType>& templateTypes);

	Share<HazeCompilerValue> GetOrCreateGlobalStringVariable(const HString& str);

	uint32 GetGlobalStringIndex(Share<HazeCompilerValue> value);

	void PreStartCreateGlobalVariable();

	void EndCreateGlobalVariable();

	Share<HazeCompilerValue> CreateGlobalVariable(const HazeDefineVariable& var, Share<HazeCompilerValue> refValue = nullptr,
		V_Array<Share<HazeCompilerValue>> arraySize = {}, V_Array<HazeDefineType>* params = nullptr);

	static Share<HazeCompilerValue> GetGlobalVariable(HazeCompilerModule* m, const HString& name);

	static bool GetGlobalVariableName(HazeCompilerModule* m, const Share<HazeCompilerValue>& value, HString& outName);

	static void GenVariableHzic(HazeCompilerModule* compilerModule, HAZE_STRING_STREAM& hss, const Share<HazeCompilerValue>& value, int index = -1);

	static Share<HazeCompilerEnum> GetEnum(HazeCompilerModule* m, const HString& name);

private:
	Share<HazeCompilerValue> CreateAdd(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateSub(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateMul(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateDiv(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateMod(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateBitAnd(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateBitOr(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateBitNeg(Share<HazeCompilerValue> value);

	Share<HazeCompilerValue> CreateBitXor(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateShl(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateShr(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, bool isAssign = false);

	Share<HazeCompilerValue> CreateNot(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right);

	Share<HazeCompilerValue> CreateNeg(Share<HazeCompilerValue> value);

	Share<HazeCompilerValue> CreateInc(Share<HazeCompilerValue> value, bool isPreInc);

	Share<HazeCompilerValue> CreateDec(Share<HazeCompilerValue> value, bool isPreDec);

	Share<HazeCompilerValue> CreateArrayInit(Share<HazeCompilerValue> array, Share<HazeCompilerValue> initList);

	Share<HazeCompilerValue> CreateFunctionCall(Share<HazeCompilerFunction> callFunction, V_Array<Share<HazeCompilerValue>>& params, Share<HazeCompilerValue> thisPointerTo = nullptr);

	Share<HazeCompilerValue> CreateFunctionCall(Share<HazeCompilerValue> pointerFunction, V_Array<Share<HazeCompilerValue>>& params, Share<HazeCompilerValue> thisPointerTo = nullptr);

	Share<HazeCompilerValue> GenIRCode_BinaryOperater(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right, InstructionOpCode opCode);

	void GenIRCode_UnaryOperator(Share<HazeCompilerValue> value, InstructionOpCode opCode);

	void GenIRCode_Ret(Share<HazeCompilerValue> value);

	void GenIRCode_Cmp(HazeCmpType cmpType, Share<HazeBaseBlock> ifJmpBlock, Share<HazeBaseBlock> elseJmpBlock);

	void GenIRCode_JmpTo(Share<HazeBaseBlock> block);

	//static void GenValueHzicText(HazeCompilerModule* Module, HAZE_STRING_STREAM& HSS, const Share<HazeCompilerValue>& Value, int Index = -1);

private:
	void FunctionCall(HAZE_STRING_STREAM& hss, const HString& callName, uint32& size, V_Array<Share<HazeCompilerValue>>& params, Share<HazeCompilerValue> thisPointerTo);

	void GenCodeFile();

	void GenICode();

private:
	Share<HazeCompilerValue> GetGlobalVariable_Internal(const HString& name);

	bool GetGlobalVariableName_Internal(const Share<HazeCompilerValue>& value, HString& outName);

	Share<HazeCompilerEnum> GetEnum_Internal(const HString& name);

private:
	HazeCompiler* m_Compiler;

	HString m_Path;

	HazeLibraryType m_ModuleLibraryType;

	HAZE_OFSTREAM m_FS_I_Code;

	HString m_CurrClass;
	HashMap<HString, Share<HazeCompilerClass>> m_HashMap_Classes;

	HString m_CurrFunction;
	HashMap<HString, Share<HazeCompilerFunction>> m_HashMap_Functions;

	HString m_CurrEnum;
	HashMap<HString, Share<HazeCompilerEnum>> m_HashMap_Enums;

	//之后考虑全局变量放在一个block里面，复用其中的函数与成员变量
	V_Array<Pair<HString, Share<HazeCompilerValue>>> m_Variables; //全局变量
	V_Array<Pair<int, int>> m_VariablesAddress;
	V_Array<HString> m_ModuleIRCodes;

	HashMap<HString, Share<HazeCompilerValue>> m_HashMap_StringTable;
	HashMap<int, const HString*> m_HashMap_StringMapping;

	HashMap<HString, TemplateCacheTextData> m_HashMap_TemplateText;

	V_Array<HazeCompilerModule*> m_ImportModules;

	bool m_IsBeginCreateFunctionVariable;
	bool m_IsGenTemplateCode;
};
