#pragma once

#include <unordered_map>
#include <fstream>

#include "HazeHeader.h"

class Compiler;
class CompilerValue;
class CompilerBlock;
class CompilerFunction;
class CompilerClosureFunction;
enum class ClassCompilerFunctionType : x_uint8;
enum class ParseStage;
class CompilerClass;
class CompilerEnum;
class HazeCompilerTemplateFunction;
class HazeCompilerTemplateClass;

class CompilerModule
{
	friend class Compiler;
	friend class CompilerSymbol;
	friend struct PushTempRegister;
private:
	struct SearchContext
	{
		HashSet<const CompilerModule*> VisitedModules;
		x_uint32 MaxDepth = 10;
		x_uint32 CurrentDepth = 0;
	};

public:

	CompilerModule(Compiler* compiler, const HString& moduleName, const HString& modulePath);

	~CompilerModule();

	Compiler* GetCompiler() { return m_Compiler; }

	bool NeedParse() const { return m_FS_I_Code != nullptr; }

	bool ParseIntermediateFile(HAZE_IFSTREAM& stream/*, const HString& moduleName*/);

	const HString& GetName() const;

	const HString& GetPath() const { return m_Path; }

	const HString& GetCurrClassName() const { return m_CurrClass; }

	HazeLibraryType GetModuleLibraryType() { return m_ModuleLibraryType; }

	void MarkLibraryType(HazeLibraryType type);

	void RestartTemplateModule(const HString& moduleName);

	void FinishModule();

	Share<CompilerClass> CreateClass(const HString& name, V_Array<CompilerClass*>& parentClass, V_Array<Pair<HString, Share<CompilerValue>>>& classData);

	Share<CompilerEnum> CreateEnum(const HString& name);

	Share<CompilerEnum> GetCurrEnum();

	const HString& GetCurrEnumName() const { return m_CurrEnum; }

	void FinishCreateEnum();

	void FinishCreateClass();

	Share<CompilerClass> GetClass(const HString& className);

	x_uint32 GetClassSize(const HString& className);

	Share<CompilerFunction> GetGlobalDataFunction() { return m_GlobalDataFunction; }

	Share<CompilerFunction> GetCurrFunction();

	Share<CompilerFunction> GetCurrClosure();

	Share<CompilerFunction> GetCurrClosureOrFunction();

	Share<CompilerFunction> GetUpOneLevelClosureOrFunction();

	Share<CompilerFunction> CreateFunction(const HString& name, const HazeVariableType& type, V_Array<HazeDefineVariable>& params);

	Share<CompilerFunction> CreateFunction(Share<CompilerClass> compilerClass, HazeFunctionDesc desc,
		const HString& name, const HazeVariableType& type, V_Array<HazeDefineVariable>& params);

	Share<CompilerClosureFunction> CreateClosureFunction(HazeVariableType& type, V_Array<HazeDefineVariable>& params);

	void BeginCreateFunctionParamVariable() { m_IsBeginCreateFunctionVariable = true; }

	void EndCreateFunctionParamVariable() { m_IsBeginCreateFunctionVariable = false; }

	void BeginGlobalDataDefine();

	void EndGlobalDataDefine();

	bool IsBeginCreateFunctionVariable() const { return m_IsBeginCreateFunctionVariable; }

	void FinishFunction();
	void FinishClosure();

	Share<CompilerFunction> GetFunction(const HString& name);

	bool IsImportModule(CompilerModule* m) const;

	CompilerModule* ExistGlobalValue(const HString& name);

	Share<CompilerValue> GetOrCreateGlobalStringVariable(const HString& str);

	x_uint32 GetGlobalStringIndex(Share<CompilerValue> value);

	Share<CompilerValue> CreateGlobalVariable(const HazeDefineVariable& var, int line, Share<CompilerValue> refValue = nullptr);

	Share<CompilerValue> GetClosureVariable(const HString& name, bool addRef);

	void ClosureAddLocalRef(Share<CompilerValue> value, const HString& name);

	static Share<CompilerValue> GetGlobalVariable(CompilerModule* m, const HString& name);

	static bool GetGlobalVariableName(CompilerModule* m, const Share<CompilerValue>& value, HString& outName, bool getOffset = false,
		V_Array<Pair<x_uint64, CompilerValue*>>* offsets = nullptr);

	static bool GetClosureVariableName(CompilerModule* m, const Share<CompilerValue>& value, HString& outName);

	static Share<CompilerEnum> GetEnum(CompilerModule* m, const HString& name);

	static Share<CompilerEnum> GetEnum(CompilerModule* m, x_uint32 typeId);

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

	Share<CompilerValue> CreateNew(const HazeVariableType& data, V_Array<Share<CompilerValue>>* countValue, Share<CompilerFunction> closure = nullptr);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerFunction> callFunction, const V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo = nullptr,
		const HString* nameSpace = nullptr);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerValue> pointerFunction, V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo = nullptr);

	Share<CompilerValue> CreateAdvanceTypeFunctionCall(struct AdvanceFunctionInfo* functionInfo, x_uint16 index, const V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo = nullptr, HazeVariableType* expectType = nullptr);

	void GenIRCode_BinaryOperater(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2, InstructionOpCode opCode, bool check = true);

	void GenIRCode_UnaryOperator(Share<CompilerValue> assignTo, Share<CompilerValue> value, InstructionOpCode opCode);

	void GenIRCode_Ret(Share<CompilerValue> value);

	void GenIRCode_Cmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock);

	void GenIRCode_JmpTo(Share<CompilerBlock> block);

private:
	void FunctionCall(HAZE_STRING_STREAM& hss, Share<CompilerFunction> callFunction, Share<CompilerValue> pointerFunction,
		AdvanceFunctionInfo* advancFunctionInfo, x_uint32& size, const V_Array<Share<CompilerValue>>& params,
		Share<CompilerValue> thisPointerTo);

	void GenCodeFile();

	void GenICode();

	//void GenICode_TypeInfo();

private:
	Share<CompilerFunction> GetFunction_Internal(const HString& name, SearchContext& context);

	CompilerModule* ExistGlobalValue_Internal(const HString& name, SearchContext& context);

	Share<CompilerValue> GetGlobalVariable_Internal(const HString& name, SearchContext& context);

	bool GetGlobalVariableName_Internal(const Share<CompilerValue>& value, HString& outName, bool getOffset,
		V_Array<Pair<x_uint64, CompilerValue*>>* offsets, SearchContext& context);

	Share<CompilerClass> GetClass_Internal(const HString& className, SearchContext& context);

	Share<CompilerEnum> GetEnum_Internal(const HString& name, SearchContext& context);

private:
	Compiler* m_Compiler;

	HString m_Path;

	HazeLibraryType m_ModuleLibraryType;

	HAZE_OFSTREAM* m_FS_I_Code;

	HString m_CurrClass;
	HashMap<HString, Share<CompilerClass>> m_HashMap_Classes;

	HString m_CurrFunction;
	HashMap<HString, Share<CompilerFunction>> m_HashMap_Functions;

	V_Array<Share<CompilerClosureFunction>> m_Closures;
	V_Array<Share<CompilerClosureFunction>> m_ClosureStack;

	HString m_CurrEnum;
	HashMap<HString, Share<CompilerEnum>> m_HashMap_Enums;

	Share<CompilerFunction> m_GlobalDataFunction;

	HashMap<HString, Share<CompilerValue>> m_HashMap_StringTable;
	HashMap<int, const HString*> m_HashMap_StringMapping;

	V_Array<CompilerModule*> m_ImportModules;

	ParseStage m_ParseStage;
	bool m_IsBeginCreateFunctionVariable;
	bool m_IsGenTemplateCode;
};
