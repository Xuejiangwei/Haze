#pragma once
#include "HazeInstruction.h"

class HazeVM;
class CompilerValue;
class HazeCompilerInitListValue;
class CompilerFunction;
class CompilerEnum;
class CompilerClass;
class CompilerModule;
class CompilerBlock;

struct AdvanceFunctionInfo
{
	void(*ClassFunc)(HAZE_STD_CALL_PARAM);
	HazeDefineType Type;
	V_Array<HazeDefineType> Params;
};

struct AdvanceClassInfo
{
	HashMap<HString, AdvanceFunctionInfo> Functions;
};

struct HashHString
{
	const HString* Str;

	bool operator== (const HashHString& p) const
	{
		return *Str == *p.Str;
	}
};

template<>
struct std::hash<HashHString>
{
	size_t operator() (const HashHString& s) const noexcept
	{
		return std::hash<HString>()(*s.Str);
	}
};

class Compiler
{
public:
	Compiler(HazeVM* vm);

	~Compiler();

	void RegisterAdvanceClassInfo(HazeValueType type, AdvanceClassInfo& info);

	void PreRegisterClass(const ClassData& data);
	void PreRegisterVariable();
	void PreRegisterFunction();

	bool InitializeCompiler(const HString& moduleName, const HString& path);

	void FinishParse();

	CompilerModule* ParseBaseModule(const HChar* moduleName, const HChar* moduleCode);

	CompilerModule* ParseModule(const HString& modulePath);

	void FinishModule();

	CompilerModule* GetModule(const HString& name);

	const HString* GetModuleName(const CompilerModule* compilerModule) const;

	const HString* GetModuleTableClassName(const HString& name);
	const HString* GetModuleTableEnumName(const HString& name);

	Unique<CompilerModule>& GetCurrModule();

	bool CurrModuleIsStdLib();

	Pair<Share<CompilerFunction>, Share<CompilerValue>> GetFunction(const HString& name);

	const HString& GetCurrModuleName() const { return m_ModuleNameStack.back(); }

	void PopCurrModule() { return m_ModuleNameStack.pop_back(); }

	void AddImportModuleToCurrModule(CompilerModule* compilerModule);

	bool IsClass(const HString& name);

	bool IsEnum(const HString& name);

	bool IsTemplateClass(const HString& name);

	void MarkParseTemplate(bool begin, const HString* moduleName = nullptr);

	Share<CompilerValue> GenConstantValue(HazeValueType type, const HazeValue& var, HazeValueType* varType = nullptr);

	Share<CompilerValue> GenStringVariable(HString& str);

	Share<CompilerValue> GetGlobalVariable(const HString& name);

	Share<CompilerValue> GetLocalVariable(const HString& name);

	Share<CompilerValue> GetEnumVariable(const HString& enumName, const HString& name);

	Share<CompilerValue> GetConstantValueInt(int v);

	Share<CompilerValue> GetConstantValueUint64(uint64 v);

	Share<CompilerValue> GenConstantValueBool(bool isTrue);

	Share<CompilerValue> GetNullPtr(const HazeDefineType& type);

	bool IsConstantValueBoolTrue(Share<CompilerValue> v);

	bool IsConstantValueBoolFalse(Share<CompilerValue> v);

public:
	Share<CompilerValue> GetTempRegister(const HazeDefineType& type);
	
	static Share<CompilerValue> GetNewRegister(CompilerModule* compilerModule, const HazeDefineType& data);

	static HashMap<const HChar*, Share<CompilerValue>> GetUseTempRegister();

	static void ClearTempRegister(const HashMap<const HChar*, Share<CompilerValue>>& useTempRegisters);

	static void ResetTempRegister(const HashMap<const HChar*, Share<CompilerValue>>& useTempRegisters);

	static Share<CompilerValue> GetRegister(const HChar* name);

	static const HChar* GetRegisterName(const Share<CompilerValue>& compilerRegister);

public:
	void SetInsertBlock(Share<CompilerBlock> block);

	Share<CompilerBlock> GetInsertBlock() { return m_InsertBaseBlock; }

	void ClearBlockPoint();

public:
	Share<CompilerValue> CreateVariableBySection(HazeSectionSignal section, Unique<CompilerModule>& mod, Share<CompilerFunction> func,
		const HazeDefineVariable& var, int line, Share<CompilerValue> refValue = nullptr, V_Array<Share<CompilerValue>> arraySize = {},
		V_Array<HazeDefineType>* params = nullptr);

	Share<CompilerValue> CreateLocalVariable(Share<CompilerFunction> Function, const HazeDefineVariable& Variable, int Line,
		Share<CompilerValue> RefValue = nullptr, V_Array<Share<CompilerValue>> m_ArraySize = {}, V_Array<HazeDefineType>* Params = nullptr);

	Share<CompilerValue> CreateGlobalVariable(Unique<CompilerModule>& m_Module, const HazeDefineVariable& Var, Share<CompilerValue> RefValue = nullptr,
		V_Array<Share<CompilerValue>> m_ArraySize = {}, V_Array<HazeDefineType>* Params = nullptr);

	Share<CompilerValue> CreateClassVariable(Unique<CompilerModule>& m_Module, const HazeDefineVariable& Var, Share<CompilerValue> RefValue = nullptr,
		V_Array<Share<CompilerValue>> m_ArraySize = {}, V_Array<HazeDefineType>* Params = nullptr);

	Share<CompilerValue> CreateLea(Share<CompilerValue> allocaValue, Share<CompilerValue> value);

	Share<CompilerValue> CreateMov(Share<CompilerValue> allocaValue, Share<CompilerValue> value, bool storeValue = true);

	Share<CompilerValue> CreateMovToPV(Share<CompilerValue> allocaValue, Share<CompilerValue> value);

	Share<CompilerValue> CreateMovPV(Share<CompilerValue> allocaValue, Share<CompilerValue> value);

	Share<CompilerValue> CreateRet(Share<CompilerValue> value);

	Share<CompilerValue> CreateAdd(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	Share<CompilerValue> CreateSub(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	Share<CompilerValue> CreateMul(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	Share<CompilerValue> CreateDiv(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	Share<CompilerValue> CreateMod(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	Share<CompilerValue> CreateBitAnd(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	Share<CompilerValue> CreateBitOr(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	Share<CompilerValue> CreateBitXor(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	Share<CompilerValue> CreateShl(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);
	Share<CompilerValue> CreateShr(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2);

	Share<CompilerValue> CreateBitNeg(Share<CompilerValue> assignTo, Share<CompilerValue> value);
	Share<CompilerValue> CreateNeg(Share<CompilerValue> assignTo, Share<CompilerValue> value);

	Share<CompilerValue> CreateNot(Share<CompilerValue> left, Share<CompilerValue> right);

	Share<CompilerValue> CreateInc(Share<CompilerValue> value, bool isPreInc);

	Share<CompilerValue> CreateDec(Share<CompilerValue> value, bool isPreDec);

	Share<CompilerValue> CreateNew(Share<CompilerFunction> function, const HazeDefineType& data, V_Array<Share<CompilerValue>>* countValue);
	
	Share<CompilerValue> CreateCast(const HazeDefineType& type, Share<CompilerValue> value);

	Share<CompilerValue> CreateCVT(Share<CompilerValue> left, Share<CompilerValue> right);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerFunction> function, V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo = nullptr);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerValue> pointerFunction, V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo = nullptr);

	Share<CompilerValue> CreateAdvanceTypeFunctionCall(HazeValueType advanceType, const HString& functionName,
		V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo);

public:
	Share<CompilerValue> CreateArrayElement(Share<CompilerValue> value, V_Array<Share<CompilerValue>> indices);

public:
	Share<CompilerValue> CreatePointerToValue(Share<CompilerValue> value);

	//Share<HazeCompilerValue> CreatePointerToArray(Share<HazeCompilerValue> arrValue, Share<HazeCompilerValue> index = nullptr);

	//Share<HazeCompilerValue> CreatePointerToArrayElement(Share<HazeCompilerValue> elementValue);

	//Share<HazeCompilerValue> CreatePointerToPointerArray(Share<HazeCompilerValue> pointerArray, Share<HazeCompilerValue> index = nullptr);

	Share<CompilerValue> CreatePointerToFunction(Share<CompilerFunction> function, Share<CompilerValue> pointer);

public:
	void CreateJmpToBlock(Share<CompilerBlock> block);

	void CreateCompareJmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock);

	Share<CompilerValue> CreateIntCmp(Share<CompilerValue> left, Share<CompilerValue> right);

	Share<CompilerValue> CreateBoolCmp(Share<CompilerValue> value);

	void ReplaceConstantValueByStrongerType(Share<CompilerValue>& left, Share<CompilerValue>& right);

public:
	void RegisterClassToSymbolTable(const HString& className);

	void OnCreateClass(Share<CompilerClass> compClass);

	const HString* GetSymbolTableNameAddress(const HString& className);

	Share<CompilerEnum> GetBaseModuleEnum(const HString& name);

	Share<CompilerValue> GetBaseModuleGlobalVariable(const HString& name);

	Share<CompilerClass> GetBaseModuleClass(const HString& className);

	bool GetBaseModuleGlobalVariableName(const Share<CompilerValue>& value, HString& outName, bool getOffset = false,
		V_Array<uint64>* offsets = nullptr);

	void GetRealTemplateTypes(const TemplateDefineTypes& types, V_Array<HazeDefineType>& defineTypes);

	void InsertLineCount(int64 lineCount);

	bool IsDebug() const;

private:
	HazeVM* m_VM;

	HashMap<HazeValueType, AdvanceClassInfo> m_AdvanceClassInfo;

	V_Array<HString> m_ModuleNameStack;

	HashMap<HString, Unique<CompilerModule>> m_CompilerModules;
	HashMap<HString, CompilerModule*> m_CompilerBaseModules;

	//³£Á¿
	HashMap<bool, Share<CompilerValue>> m_BoolConstantValues;

	HashMap<int8, Share<CompilerValue>> m_Int8_ConstantValues;
	HashMap<uint8, Share<CompilerValue>> m_UInt8_ConstantValues;
	HashMap<int16, Share<CompilerValue>> m_Int16_ConstantValues;
	HashMap<uint16, Share<CompilerValue>> m_UInt16_ConstantValues;
	HashMap<int32, Share<CompilerValue>> m_Int32_ConstantValues;
	HashMap<uint32, Share<CompilerValue>> m_UInt32_ConstantValues;
	HashMap<int64, Share<CompilerValue>> m_Int64_ConstantValues;
	HashMap<uint64, Share<CompilerValue>> m_UInt64_ConstantValues;

	HashMap<float32, Share<CompilerValue>> m_Float32_ConstantValues;
	HashMap<float64, Share<CompilerValue>> m_Float64_ConstantValues;

	List<HString> m_CacheSymbols;
	HashMap<HashHString, Share<CompilerClass>> m_SymbolTable;

	//BaseBlock
	Share<CompilerBlock> m_InsertBaseBlock;
};
