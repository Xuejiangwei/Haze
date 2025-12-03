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

	CompilerModule(Compiler* compiler, const STDString& moduleName, const STDString& modulePath);

	~CompilerModule();

	Compiler* GetCompiler() { return m_Compiler; }

	bool NeedParse() const { return m_FS_I_Code != nullptr; }

	bool ParseIntermediateFile(HAZE_IFSTREAM& stream, const STDString& moduleName);

	const STDString& GetName() const;

	const STDString& GetPath() const { return m_Path; }

	const STDString& GetCurrClassName() const { return m_CurrClass; }

	HazeLibraryType GetModuleLibraryType() { return m_ModuleLibraryType; }

	void MarkLibraryType(HazeLibraryType type);

	void RestartTemplateModule(const STDString& moduleName);

	void FinishModule();

	Share<CompilerClass> CreateClass(const STDString& name, V_Array<CompilerClass*>& parentClass, V_Array<Pair<STDString, Share<CompilerValue>>>& classData);

	Share<CompilerEnum> CreateEnum(const STDString& name);

	Share<CompilerEnum> GetCurrEnum();

	const STDString& GetCurrEnumName() const { return m_CurrEnum; }

	void FinishCreateEnum();

	void FinishCreateClass();

	Share<CompilerClass> GetClass(const STDString& className);

	x_uint32 GetClassSize(const STDString& className);

	Share<CompilerFunction> GetGlobalDataFunction() { return m_GlobalDataFunction; }

	Share<CompilerFunction> GetCurrFunction();

	Share<CompilerFunction> GetCurrClosure();

	Share<CompilerFunction> GetCurrClosureOrFunction();

	Share<CompilerFunction> GetUpOneLevelClosureOrFunction();

	Share<CompilerFunction> CreateFunction(const STDString& name, const HazeVariableType& type, V_Array<HazeDefineVariable>& params);

	Share<CompilerFunction> CreateFunction(Share<CompilerClass> compilerClass, HazeFunctionDesc desc,
		const STDString& name, const HazeVariableType& type, V_Array<HazeDefineVariable>& params);

	Share<CompilerClosureFunction> CreateClosureFunction(HazeVariableType& type, V_Array<HazeDefineVariable>& params);

	void BeginCreateFunctionParamVariable(x_int8 index) { m_BeginCreateFunctionParamVariableIndex = index; }

	void EndCreateFunctionParamVariable() { m_BeginCreateFunctionParamVariableIndex = -1; }

	bool IsBeginCreateFunctionParamVariable() const { return m_BeginCreateFunctionParamVariableIndex >= 0; }

	x_int8 GetCreateFunctionParamVariable() const { return m_BeginCreateFunctionParamVariableIndex; }

	void BeginCreateClassVariable(x_int32 index) { m_BeginCreateClassVariableIndex = index; }

	void EndCreateClassVariable() { m_BeginCreateClassVariableIndex = -1; }

	bool IsBeginCreateClassVariable() const { return m_BeginCreateClassVariableIndex >= 0; }

	x_int32 GetCreateClassVariable() const { return m_BeginCreateClassVariableIndex; }

	void BeginGlobalDataDefine();

	void EndGlobalDataDefine();

	void FinishFunction();
	void FinishClosure();

	Share<CompilerFunction> GetFunction(const STDString& name);

	bool IsImportModule(CompilerModule* m) const;

	CompilerModule* ExistGlobalValue(const STDString& name);

	Share<CompilerValue> GetOrCreateGlobalStringVariable(const STDString& str);

	x_uint32 GetGlobalStringIndex(Share<CompilerValue> value);

	Share<CompilerValue> CreateGlobalVariable(const HazeDefineVariable& var, int line, Share<CompilerValue> refValue = nullptr);

	Share<CompilerValue> GetClosureVariable(const STDString& name, bool addRef);

	void ClosureAddLocalRef(Share<CompilerValue> value, const STDString& name);

	static Share<CompilerValue> GetGlobalVariable(CompilerModule* m, const STDString& name);

	static bool GetGlobalVariableName(CompilerModule* m, const Share<CompilerValue>& value, HStringView& outName);
	static bool GetGlobalVariableId(CompilerModule* m, const Share<CompilerValue>& value, InstructionOpId& outId);

	static bool GetClosureVariableName(CompilerModule* m, const Share<CompilerValue>& value, HStringView& outName);
	static bool GetClosureVariableId(CompilerModule* m, const Share<CompilerValue>& value, InstructionOpId& outId);

	static Share<CompilerEnum> GetEnum(CompilerModule* m, const STDString& name);

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

	Share<CompilerValue> CreateFunctionCall(Share<CompilerFunction> callFunction, const V_Array<Share<CompilerValue>>& params, bool pushParam, Share<CompilerValue> thisPointerTo = nullptr,
		const STDString* nameSpace = nullptr);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerValue> pointerFunction, V_Array<Share<CompilerValue>>& params, bool pushParam, Share<CompilerValue> thisPointerTo = nullptr);

	Share<CompilerValue> CreateAdvanceTypeFunctionCall(struct AdvanceFunctionInfo* functionInfo, x_uint16 index, const V_Array<Share<CompilerValue>>& params, bool pushParam, Share<CompilerValue> thisPointerTo = nullptr, HazeVariableType* expectType = nullptr);

	void GenIRCode_BinaryOperater(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2, InstructionOpCode opCode, bool check = true);

	void GenIRCode_UnaryOperator(Share<CompilerValue> assignTo, Share<CompilerValue> value, InstructionOpCode opCode);

	void GenIRCode_Ret(Share<CompilerValue> value);

	void GenIRCode_Cmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock);

	void GenIRCode_JmpTo(Share<CompilerBlock> block);

private:
	void FunctionCall(HAZE_STRING_STREAM& hss, Share<CompilerFunction> callFunction, Share<CompilerValue> pointerFunction,
		AdvanceFunctionInfo* advancFunctionInfo, x_uint32& size, const V_Array<Share<CompilerValue>>& params,
		Share<CompilerValue> thisPointerTo, bool pushParam);

	void GenCodeFile();

	void GenICode();

	//void GenICode_TypeInfo();

private:
	Share<CompilerFunction> GetFunction_Internal(const STDString& name, SearchContext& context);

	CompilerModule* ExistGlobalValue_Internal(const STDString& name, SearchContext& context);

	Share<CompilerValue> GetGlobalVariable_Internal(const STDString& name, SearchContext& context);

	bool GetGlobalVariableName_Internal(const Share<CompilerValue>& value, HStringView& outName, SearchContext& context);
	bool GetGlobalVariableId_Internal(const Share<CompilerValue>& value, InstructionOpId& outId, SearchContext& context);

	Share<CompilerClass> GetClass_Internal(const STDString& className, SearchContext& context);

	Share<CompilerEnum> GetEnum_Internal(const STDString& name, SearchContext& context);

private:
	Compiler* m_Compiler;

	STDString m_Path;

	HazeLibraryType m_ModuleLibraryType;

	HAZE_OFSTREAM* m_FS_I_Code;

	STDString m_CurrClass;
	HashMap<STDString, Share<CompilerClass>> m_HashMap_Classes;

	STDString m_CurrFunction;
	HashMap<STDString, Share<CompilerFunction>> m_HashMap_Functions;

	V_Array<Share<CompilerClosureFunction>> m_Closures;
	V_Array<Share<CompilerClosureFunction>> m_ClosureStack;

	STDString m_CurrEnum;
	HashMap<STDString, Share<CompilerEnum>> m_HashMap_Enums;

	Share<CompilerFunction> m_GlobalDataFunction;

	HashMap<STDString, Share<CompilerValue>> m_HashMap_StringTable;
	HashMap<int, HStringView> m_HashMap_StringMapping;

	V_Array<CompilerModule*> m_ImportModules;

	ParseStage m_ParseStage;
	x_int32 m_BeginCreateClassVariableIndex;
	x_int8 m_BeginCreateFunctionParamVariableIndex;
	bool m_IsGenTemplateCode;
};
