#pragma once

#include <unordered_map>
#include <fstream>

#include "HazeHeader.h"

class Compiler;
class CompilerValue;
class CompilerBlock;
class CompilerFunction;
class CompilerClass;
class CompilerEnum;
class HazeCompilerTemplateFunction;
class HazeCompilerTemplateClass;

struct TemplateCacheTextData
{
	uint32 StartLine;
	HString Text;
	V_Array<HString> Types;
};

class CompilerModule
{
	friend class Compiler;
	friend struct PushTempRegister;
public:

	CompilerModule(Compiler* compiler, const HString& moduleName, const HString& modulePath);

	~CompilerModule();

	Compiler* GetCompiler() { return m_Compiler; }

	const HString& GetName() const;

	const HString& GetPath() const { return m_Path; }

	const HString& GetCurrClassName() const { return m_CurrClass; }

	HazeLibraryType GetModuleLibraryType() { return m_ModuleLibraryType; }

	void MarkLibraryType(HazeLibraryType type);

	void RestartTemplateModule(const HString& moduleName);

	void FinishModule();

	Share<CompilerClass> CreateClass(const HString& name, V_Array<CompilerClass*>& parentClass,
		V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<CompilerValue>>>>>& classData);

	Share<CompilerEnum> CreateEnum(const HString& name, HazeValueType baseType);

	Share<CompilerEnum> GetCurrEnum();
	
	const HString& GetCurrEnumName() const { return m_CurrEnum; }

	void FinishCreateEnum();

	void FinishCreateClass();

	Share<CompilerClass> GetClass(const HString& className);

	uint32 GetClassSize(const HString& className);

	Share<CompilerFunction> GetCurrFunction();

	Share<CompilerFunction> CreateFunction(const HString& name, HazeDefineType& type, V_Array<HazeDefineVariable>& params);

	Share<CompilerFunction> CreateFunction(Share<CompilerClass> compilerClass, const HString& name, HazeDefineType& type, V_Array<HazeDefineVariable>& params);

	void BeginCreateFunctionParamVariable() { m_IsBeginCreateFunctionVariable = true; }

	void EndCreateFunctionParamVariable() { m_IsBeginCreateFunctionVariable = false; }

	bool IsBeginCreateFunctionVariable() const { return m_IsBeginCreateFunctionVariable; }

	void FinishFunction();

	Share<CompilerFunction> GetFunction(const HString& name);

	void StartCacheTemplate(HString& templateName, uint32 startLine, HString& templateText, V_Array<HString>& templateTypes);

	bool IsTemplateClass(const HString& name);

	bool ResetTemplateClassRealName(HString& inName, const V_Array<HazeDefineType>& templateTypes);

	Share<CompilerValue> GetOrCreateGlobalStringVariable(const HString& str);

	uint32 GetGlobalStringIndex(Share<CompilerValue> value);

	void PreStartCreateGlobalVariable();

	void EndCreateGlobalVariable();

	Share<CompilerValue> CreateGlobalVariable(const HazeDefineVariable& var, Share<CompilerValue> refValue = nullptr,
		uint64 arrayDimension = 0, V_Array<HazeDefineType>* params = nullptr);

	static Share<CompilerValue> GetGlobalVariable(CompilerModule* m, const HString& name);

	static bool GetGlobalVariableName(CompilerModule* m, const Share<CompilerValue>& value, HString& outName, bool getOffset = false,
		V_Array<Pair<uint64, CompilerValue*>>* offsets = nullptr);

	static Share<CompilerEnum> GetEnum(CompilerModule* m, const HString& name);

	void PushModuleIRCode(const HString& irCode);

private:
	void CreateAdd(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	void CreateSub(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	void CreateMul(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	void CreateDiv(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	void CreateMod(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	void CreateBitAnd(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	void CreateBitOr(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	void CreateBitXor(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	void CreateShl(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	void CreateShr(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);

	void CreateBitNeg(Share<CompilerValue> assignTo, Share<CompilerValue> oper1);
	void CreateNeg(Share<CompilerValue> assignTo, Share<CompilerValue> oper1);
	void CreateNot(Share<CompilerValue> assignTo, Share<CompilerValue> oper1);
	Share<CompilerValue> CreateInc(Share<CompilerValue> value, bool isPreInc);
	Share<CompilerValue> CreateDec(Share<CompilerValue> value, bool isPreDec);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerFunction> callFunction, V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo = nullptr);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerValue> pointerFunction, V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo = nullptr);
	
	Share<CompilerValue> CreateAdvanceTypeFunctionCall(struct AdvanceFunctionInfo& functionInfo, V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo = nullptr);

	void GenIRCode_BinaryOperater(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2, InstructionOpCode opCode, bool check = true);

	void GenIRCode_UnaryOperator(Share<CompilerValue> assignTo, Share<CompilerValue> value, InstructionOpCode opCode);

	void GenIRCode_Ret(Share<CompilerValue> value);

	void GenIRCode_Cmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock);

	void GenIRCode_JmpTo(Share<CompilerBlock> block);

private:
	void FunctionCall(HAZE_STRING_STREAM& hss, Share<CompilerFunction> callFunction, Share<CompilerValue> pointerFunction, 
		AdvanceFunctionInfo* advancFunctionInfo, uint32& size, V_Array<Share<CompilerValue>>& params, 
		Share<CompilerValue> thisPointerTo);

	void GenCodeFile();

	void GenICode();

private:
	Share<CompilerValue> GetGlobalVariable_Internal(const HString& name);

	bool GetGlobalVariableName_Internal(const Share<CompilerValue>& value, HString& outName, bool getOffset, 
		V_Array<Pair<uint64, CompilerValue*>>* offsets);

	Share<CompilerEnum> GetEnum_Internal(const HString& name);

private:
	Compiler* m_Compiler;

	HString m_Path;

	HazeLibraryType m_ModuleLibraryType;

	HAZE_OFSTREAM m_FS_I_Code;

	HString m_CurrClass;
	HashMap<HString, Share<CompilerClass>> m_HashMap_Classes;

	HString m_CurrFunction;
	HashMap<HString, Share<CompilerFunction>> m_HashMap_Functions;

	HString m_CurrEnum;
	HashMap<HString, Share<CompilerEnum>> m_HashMap_Enums;

	//之后考虑全局变量放在一个block里面，复用其中的函数与成员变量
	V_Array<Pair<HString, Share<CompilerValue>>> m_Variables; //全局变量
	V_Array<Pair<int, int>> m_VariablesAddress;
	V_Array<HString> m_ModuleIRCodes;

	HashMap<HString, Share<CompilerValue>> m_HashMap_StringTable;
	HashMap<int, const HString*> m_HashMap_StringMapping;

	HashMap<HString, TemplateCacheTextData> m_HashMap_TemplateText;

	V_Array<CompilerModule*> m_ImportModules;

	bool m_IsBeginCreateFunctionVariable;
	bool m_IsGenTemplateCode;
};
