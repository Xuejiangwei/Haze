#pragma once
#include "HazeInstruction.h"

class HazeVM;
class CompilerValue;
class CompilerElementValue;
class HazeCompilerInitListValue;
class CompilerFunction;
class CompilerEnum;
class CompilerClass;
class CompilerModule;
class CompilerBlock;
class HazeTypeInfoMap;

struct AdvanceFunctionInfo
{
	void(*ClassFunc)(HAZE_OBJECT_CALL_PARAM);
	HazeVariableType Type;
	V_Array<HazeVariableType> Params;
};

struct AdvanceClassInfo
{
	V_Array<AdvanceFunctionInfo> Functions;
	HashMap<HString, x_int16> FunctionMapping;

	void Add(const HString& name, const AdvanceFunctionInfo& info)
	{
		if (FunctionMapping.find(name) == FunctionMapping.end())
		{
			FunctionMapping[name] = (x_int16)Functions.size();
			Functions.push_back(info);
		}
	}
};

struct AdvanceClassIndexInfo
{
	AdvanceClassInfo* Info = nullptr;
	x_int16 StartIndex = 0;

	const x_int16 GetIndex(const HString& name) const
	{
		auto it = Info->FunctionMapping.find(name);
		if (it != Info->FunctionMapping.end())
		{
			return StartIndex + it->second;
		}

		return -1;
	}

	Pair<AdvanceFunctionInfo*, x_int16> GetInfoAndIndex(const HString& name)
	{
		auto it = Info->FunctionMapping.find(name);
		if (it != Info->FunctionMapping.end())
		{
			return { &(Info->Functions[it->second]), (x_int16)(StartIndex + it->second) };
		}

		return { nullptr, (x_int16)-1 };
	}
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

	void RegisterAdvanceClassInfo(HazeValueType type, AdvanceClassIndexInfo info);

	void PreRegisterClass(const ClassData& data);
	void PreRegisterVariable();
	void PreRegisterFunction();

	bool InitializeCompiler(const HString& moduleName, const HString& path);

	void FinishParse();

	CompilerModule* ParseBaseModule(const HString& moduleName);

	CompilerModule* ParseModuleByImportPath(const HString& importPath);

	CompilerModule* ParseModuleByPath(const HString& modulePath);

	void ParseTypeInfoFile();

	void FinishModule();

	CompilerModule* GetModule(const HString& name);
	
	CompilerModule* GetModuleAndTryParseIntermediateFile(const HString& filePath);

	const HString* GetModuleName(const CompilerModule* compilerModule) const;

	const HString* GetModuleTableClassName(const HString& name);
	x_uint32 GetModuleTableEnumTypeId(const HString& name);

	Unique<CompilerModule>& GetCurrModule();

	bool CurrModuleIsStdLib();

	Share<CompilerFunction> GetFunction(const HString& name);

	const HString& GetCurrModuleName() const { return m_ModuleNameStack.back(); }

	void PopCurrModule() { return m_ModuleNameStack.pop_back(); }

	void AddImportModuleToCurrModule(CompilerModule* compilerModule);

	bool IsClass(const HString& name);

	bool IsEnum(const HString& name);

	void MarkParseTemplate(bool begin, const HString* moduleName = nullptr);

	Share<CompilerValue> GenConstantValue(HazeValueType type, const HazeValue& var, HazeValueType* varType = nullptr);

	Share<CompilerValue> GenStringVariable(const HString& str);

	Share<CompilerValue> GetGlobalVariable(const HString& name);

	Share<CompilerValue> GetLocalVariable(const HString& name, HString* nameSpace = nullptr);

	Share<CompilerValue> GetClosureVariable(const HString& name, bool addRef = true);

	Share<CompilerValue> GetEnumVariable(const HString& enumName, const HString& name);

	Share<CompilerValue> GetConstantValueInt(int v);

	Share<CompilerValue> GetConstantValueUint64(x_uint64 v);

	Share<CompilerValue> GenConstantValueBool(bool isTrue);

	Share<CompilerValue> GetNullPtr(const HazeVariableType& type);

	bool IsConstantValueBoolTrue(Share<CompilerValue> v);

	bool IsConstantValueBoolFalse(Share<CompilerValue> v);

public:
	Share<CompilerValue> GetTempRegister(Share<CompilerValue> v);
	Share<CompilerValue> GetTempRegister(const CompilerValue* v);
	Share<CompilerValue> GetTempRegister(const HazeVariableType& type);
	
	//static Share<CompilerValue> GetNewRegister(CompilerModule* compilerModule, const HazeDefineType& data);

	static HashMap<const x_HChar*, Share<CompilerValue>> GetUseTempRegister();

	static void ClearTempRegister(const HashMap<const x_HChar*, Share<CompilerValue>>& useTempRegisters);

	static void ResetTempRegister(const HashMap<const x_HChar*, Share<CompilerValue>>& useTempRegisters);

	static Share<CompilerValue> GetRegister(const x_HChar* name);
	static Share<CompilerValue> GetRetRegister(HazeValueType baseType, x_uint32 typeId);

	static const x_HChar* GetRegisterName(const Share<CompilerValue>& compilerRegister);

public:
	void SetInsertBlock(Share<CompilerBlock> block);

	Share<CompilerBlock> GetInsertBlock();

	Share<CompilerBlock> GetFunctionInsertBlock();

	void ClearBlockPoint();

	bool IsCompileError() const;
	void MarkCompilerError();

	bool IsNewCode() const;
	void MarkNewCode();

public:
	Share<CompilerValue> CreateVariableBySection(HazeSectionSignal section, Unique<CompilerModule>& mod, Share<CompilerFunction> func,
		const HazeDefineVariable& var, int line, Share<CompilerValue> refValue = nullptr, x_uint32 typeId = 0);

	Share<CompilerValue> CreateLocalVariable(Share<CompilerFunction> Function, const HazeDefineVariable& Variable, int Line,
		Share<CompilerValue> RefValue = nullptr);

	Share<CompilerValue> CreateGlobalVariable(Unique<CompilerModule>& m_Module, const HazeDefineVariable& Var, int Line, Share<CompilerValue> RefValue = nullptr);

	Share<CompilerValue> CreateClassVariable(CompilerModule* m_Module, const HazeVariableType& Var, Share<CompilerValue> RefValue = nullptr, TemplateDefineTypes* Params = nullptr);

	Share<CompilerValue> CreateLea(Share<CompilerValue> allocaValue, Share<CompilerValue> value);

	Share<CompilerValue> CreateMov(Share<CompilerValue> allocaValue, Share<CompilerValue> value, bool checkType = true);

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

	Share<CompilerValue> CreateBitNeg(Share<CompilerValue> assignTo, Share<CompilerValue> oper1);
	Share<CompilerValue> CreateNeg(Share<CompilerValue> assignTo, Share<CompilerValue> oper1);
	Share<CompilerValue> CreateNot(Share<CompilerValue> assignTo, Share<CompilerValue> oper1);

	Share<CompilerValue> CreateInc(Share<CompilerValue> value, bool isPreInc);

	Share<CompilerValue> CreateDec(Share<CompilerValue> value, bool isPreDec);

	Share<CompilerValue> CreateNew(const HazeVariableType& data, V_Array<Share<CompilerValue>>* countValue, Share<CompilerFunction> closure = nullptr);
	
	Share<CompilerValue> CreateCast(const HazeVariableType& type, Share<CompilerValue> value);

	Share<CompilerValue> CreateCVT(Share<CompilerValue> left, Share<CompilerValue> right);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerFunction> function, const V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo = nullptr,
		const HString* nameSpace = nullptr);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerValue> pointerFunction, V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo = nullptr);

	Share<CompilerValue> CreateAdvanceTypeFunctionCall(HazeValueType advanceType, const HString& functionName, const V_Array<Share<CompilerValue>>& param,
		Share<CompilerValue> thisPointerTo, HazeVariableType* expectType = nullptr);

	Share<CompilerValue> CreateGetAdvanceElement(Share<CompilerElementValue> element);
	Share<CompilerValue> CreateSetAdvanceElement(Share<CompilerElementValue> element, Share<CompilerValue> assignValue);

	Share<CompilerValue> CreateGetArrayElement(Share<CompilerValue> arrayValue, Share<CompilerValue> index);
	Share<CompilerValue> CreateSetArrayElement(Share<CompilerValue> arrayValue, Share<CompilerValue> index, Share<CompilerValue> assignValue);

	Share<CompilerValue> CreateGetClassMember(Share<CompilerValue> classValue, const HString& memberName);
	//Share<CompilerValue> CreateSetClassMember(Share<CompilerValue> classValue, const HString& memberName, Share<CompilerValue> assignValue);
	Share<CompilerValue> CreateGetClassMember(Share<CompilerValue> classValue, Share<CompilerValue> member);
	Share<CompilerValue> CreateSetClassMember(Share<CompilerValue> classValue, Share<CompilerValue> member, Share<CompilerValue> assignValue);

	Share<CompilerValue> CreateGetDynamicClassMember(Share<CompilerValue> classValue, const HString& memberName);
	Share<CompilerValue> CreateSetDynamicClassMember(Share<CompilerValue> classValue, const HString& memberName, Share<CompilerValue> assignValue);
	Share<CompilerValue> CreateDynamicClassFunctionCall(Share<CompilerValue> classValue, const HString& functionName, const V_Array<Share<CompilerValue>>& params);

public:
	Share<CompilerValue> CreateElementValue(Share<CompilerValue> parentValue, Share<CompilerValue> elementValue);
	Share<CompilerValue> CreateElementValue(Share<CompilerValue> parentValue, const HString& memberName);
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

	Share<CompilerValue> CreateFunctionRet(const HazeVariableType& type);

	void ReplaceConstantValueByStrongerType(Share<CompilerValue>& left, Share<CompilerValue>& right);

public:
	void RegisterClassToSymbolTable(const HString& className);

	void OnCreateClass(Share<CompilerClass> compClass);

	const HString* GetSymbolTableNameAddress(const HString& className);

	x_uint32 GetSymbolTableNameTypeId(const HString& className) const;

	AdvanceFunctionInfo* GetAdvanceFunctionInfo(HazeValueType advanceType, const HString& name);

	Share<CompilerEnum> GetBaseModuleEnum(const HString& name);
	Share<CompilerEnum> GetBaseModuleEnum(x_uint32 typeId);

	Share<CompilerValue> GetBaseModuleGlobalVariable(const HString& name);

	Share<CompilerClass> GetBaseModuleClass(const HString& className);

	bool GetBaseModuleGlobalVariableName(const Share<CompilerValue>& value, HString& outName, bool getOffset = false,
		V_Array<Pair<x_uint64, CompilerValue*>>* = nullptr);

	//void GetRealTemplateTypes(const TemplateDefineTypes& types, V_Array<HazeDefineType>& defineTypes);

	HazeTypeInfoMap* GetTypeInfoMap() { return m_TypeInfoMap.get(); }

	void InsertLineCount(x_int64 lineCount);

	bool IsDebug() const;

private:
	HazeVM* m_VM;

	HashMap<HazeValueType, AdvanceClassIndexInfo> m_AdvanceClassIndexInfo;

	V_Array<HString> m_ModuleNameStack;

	HashMap<HString, Unique<CompilerModule>> m_CompilerModules;
	HashMap<HString, CompilerModule*> m_CompilerBaseModules;

	HashMap<bool, Share<CompilerValue>> m_BoolConstantValues;

	HashMap<x_int8, Share<CompilerValue>> m_Int8_ConstantValues;
	HashMap<x_uint8, Share<CompilerValue>> m_UInt8_ConstantValues;
	HashMap<x_int16, Share<CompilerValue>> m_Int16_ConstantValues;
	HashMap<x_uint16, Share<CompilerValue>> m_UInt16_ConstantValues;
	HashMap<x_int32, Share<CompilerValue>> m_Int32_ConstantValues;
	HashMap<x_uint32, Share<CompilerValue>> m_UInt32_ConstantValues;
	HashMap<x_int64, Share<CompilerValue>> m_Int64_ConstantValues;
	HashMap<x_uint64, Share<CompilerValue>> m_UInt64_ConstantValues;

	HashMap<x_float32, Share<CompilerValue>> m_Float32_ConstantValues;
	HashMap<x_float64, Share<CompilerValue>> m_Float64_ConstantValues;

	//List<HString> m_CacheSymbols;
	HashMap<HashHString, Pair<Share<CompilerClass>, x_uint32>> m_SymbolTable;

	//BaseBlock
	Share<CompilerBlock> m_InsertBaseBlock;

	Unique<HazeTypeInfoMap> m_TypeInfoMap;

	bool m_MarkError;
	bool m_MarkNewCode;
};
