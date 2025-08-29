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
class CompilerSymbol;

struct AdvanceFunctionInfo
{
	void(*ClassFunc)(HAZE_OBJECT_CALL_PARAM);
	HazeVariableType Type;
	V_Array<HazeVariableType> Params;
};

struct AdvanceClassInfo
{
	V_Array<AdvanceFunctionInfo> Functions;
	HashMap<STDString, x_int16> FunctionMapping;

	void Add(const STDString& name, const AdvanceFunctionInfo& info)
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

	const x_int16 GetIndex(const STDString& name) const
	{
		auto it = Info->FunctionMapping.find(name);
		if (it != Info->FunctionMapping.end())
		{
			return StartIndex + it->second;
		}

		return -1;
	}

	Pair<AdvanceFunctionInfo*, x_int16> GetInfoAndIndex(const STDString& name)
	{
		auto it = Info->FunctionMapping.find(name);
		if (it != Info->FunctionMapping.end())
		{
			return { &(Info->Functions[it->second]), (x_int16)(StartIndex + it->second) };
		}

		return { nullptr, (x_int16)-1 };
	}
};

//struct HashHString
//{
//	const HString* Str;
//
//	bool operator== (const HashHString& p) const
//	{
//		return *Str == *p.Str;
//	}
//};
//
//template<>
//struct std::hash<HashHString>
//{
//	size_t operator() (const HashHString& s) const noexcept
//	{
//		return std::hash<HString>()(s.Str);
//	}
//};

enum class ParseStage
{
	// 此阶段不要生成AST
	RegisterSymbol,

	// 此阶段能过正确解析
	ParseAll,

	Finish
};

class Compiler
{
	enum class ModuleParseInterState
	{
		NotParsed,
		Parsing,
		Parsed,
		Failed
	};

	struct ModuleParseInfo
	{
		ModuleParseInterState State = ModuleParseInterState::NotParsed;
		CompilerModule* Module = nullptr;
		V_Array<STDString> ParseStack;
		x_uint32 ParseDepth = 0;
		static const int MAX_PARSE_DEPTH = 10;
	};

public:
	Compiler(HazeVM* vm);

	~Compiler();

	void CollectAllModule();

	void RegisterAdvanceClassInfo(HazeValueType type, AdvanceClassIndexInfo info);

	//void PreRegisterClass(const ClassData& data);
	void PreRegisterVariable();
	void PreRegisterFunction();

	bool InitializeCompiler(const STDString& moduleName, const STDString& path);

	void FinishParse();

	CompilerModule* ParseBaseModule(const STDString& moduleName);

	CompilerModule* ParseModuleByImportPath(const STDString& importPath);

	CompilerModule* ParseModuleByPath(const STDString& modulePath);

	void ParseTypeInfoFile();

	void FinishModule();

	CompilerModule* GetModule(const STDString& name);

	CompilerModule* GetModuleAndTryParseIntermediateFile(const STDString& filePath);

	const STDString* GetModuleName(const CompilerModule* compilerModule) const;

	const STDString* GetModuleTableClassName(const STDString& name);
	x_uint32 GetModuleTableEnumTypeId(const STDString& name);

	Unique<CompilerModule>& GetCurrModule();

	bool CurrModuleIsStdLib();

	Share<CompilerFunction> GetFunction(const STDString& name);

	const STDString& GetCurrModuleName() const { return m_ModuleNameStack.back(); }

	void PopCurrModule() { return m_ModuleNameStack.pop_back(); }

	void AddImportModuleToCurrModule(CompilerModule* compilerModule);

	bool IsClass(const STDString& name);

	bool IsEnum(const STDString& name);

	void MarkParseTemplate(bool begin, const STDString* moduleName = nullptr);

	Share<CompilerValue> GenConstantValue(HazeValueType type, const HazeValue& var, HazeValueType* varType = nullptr);

	Share<CompilerValue> GenStringVariable(const STDString& str);

	Share<CompilerValue> GetGlobalVariable(const STDString& name);

	Share<CompilerValue> GetLocalVariable(const STDString& name, STDString* nameSpace = nullptr);

	Share<CompilerValue> GetClosureVariable(const STDString& name, bool addRef = true);

	Share<CompilerValue> GetEnumVariable(const STDString& enumName, const STDString& name);

	Share<CompilerValue> GetConstantValueInt(int v);

	Share<CompilerValue> GetConstantValueUint64(x_uint64 v);

	Share<CompilerValue> GenConstantValueBool(bool isTrue);

	Share<CompilerValue> GetNullPtr();

	bool IsConstantValueBoolTrue(Share<CompilerValue> v);

	bool IsConstantValueBoolFalse(Share<CompilerValue> v);

	// 寄存器
public:
	Share<CompilerValue> GetTempRegister(Share<CompilerValue> v);
	Share<CompilerValue> GetTempRegister(const CompilerValue* v);
	Share<CompilerValue> GetTempRegister(const HazeVariableType& type);

	Share<CompilerValue> GetRegister(HazeDataDesc desc);
	Share<CompilerValue> GetRetRegister(HazeValueType baseType, x_uint32 typeId);

	const x_HChar* GetRegisterName(const Share<CompilerValue>& compilerRegister);
	x_uint64 GetRegisterIndex(const Share<CompilerValue>& compilerRegister);

	// 编译块
public:
	void SetInsertBlock(Share<CompilerBlock> block);

	Share<CompilerBlock> GetInsertBlock();

	Share<CompilerBlock> GetFunctionInsertBlock();

	void ClearBlockPoint();

	bool IsCompileError() const;
	void MarkCompilerError();

	bool IsNewCode() const;
	void MarkNewCode();

	// 创建变量
public:
	Share<CompilerValue> CreateVariableBySection(HazeSectionSignal section, Unique<CompilerModule>& mod, Share<CompilerFunction> func,
		const HazeDefineVariable& var, int line, Share<CompilerValue> refValue = nullptr);

	Share<CompilerValue> CreateLocalVariable(Share<CompilerFunction> Function, const HazeDefineVariable& Variable, int Line, Share<CompilerValue> RefValue = nullptr);
	Share<CompilerValue> CreateGlobalVariable(Unique<CompilerModule>& m_Module, const HazeDefineVariable& Var, int Line, Share<CompilerValue> RefValue = nullptr);

	Share<CompilerValue> CreateClassVariable(CompilerModule* m_Module, const HazeVariableType& Var, Share<CompilerValue> RefValue = nullptr, TemplateDefineTypes* Params = nullptr);

	Share<CompilerElementValue> CreateElementValue(Share<CompilerValue> parentValue, Share<CompilerValue> elementValue);
	Share<CompilerValue> CreateElementValue(Share<CompilerValue> parentValue, const STDString& memberName);

	Share<CompilerValue> CreatePointerToValue(Share<CompilerValue> value);
	Share<CompilerValue> CreatePointerToFunction(Share<CompilerFunction> function, Share<CompilerValue> pointer);

	void ReplaceConstantValueByStrongerType(Share<CompilerValue>& left, Share<CompilerValue>& right);

	// 创建字节码
public:
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

	void CreatePush(Share<CompilerValue> value);

	Share<CompilerValue> CreateFunctionRet(const HazeVariableType& type);

	// 以下是创建函数调用字节码
	Share<CompilerValue> CreateFunctionCall(Share<CompilerFunction> function, const V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo = nullptr,
		const STDString* nameSpace = nullptr);

	Share<CompilerValue> CreateFunctionCall(Share<CompilerValue> pointerFunction, V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo = nullptr);

	Share<CompilerValue> CreateAdvanceTypeFunctionCall(HazeValueType advanceType, const STDString& functionName, const V_Array<Share<CompilerValue>>& param,
		Share<CompilerValue> thisPointerTo, HazeVariableType* expectType = nullptr);

	Share<CompilerValue> CreateGetAdvanceElement(Share<CompilerElementValue> element);
	Share<CompilerValue> CreateSetAdvanceElement(Share<CompilerElementValue> element, Share<CompilerValue> assignValue);

	Share<CompilerValue> CreateGetArrayElement(Share<CompilerValue> arrayValue, Share<CompilerValue> index);
	Share<CompilerValue> CreateSetArrayElement(Share<CompilerValue> arrayValue, Share<CompilerValue> index, Share<CompilerValue> assignValue);

	Share<CompilerValue> CreateGetClassMember(Share<CompilerValue> classValue, const STDString& memberName);
	Share<CompilerValue> CreateGetClassMember(Share<CompilerValue> classValue, Share<CompilerValue> member);
	Share<CompilerValue> CreateSetClassMember(Share<CompilerValue> classValue, Share<CompilerValue> member, Share<CompilerValue> assignValue);

	Share<CompilerValue> CreateGetDynamicClassMember(Share<CompilerValue> classValue, const STDString& memberName);
	Share<CompilerValue> CreateSetDynamicClassMember(Share<CompilerValue> classValue, const STDString& memberName, Share<CompilerValue> assignValue);
	Share<CompilerValue> CreateDynamicClassFunctionCall(Share<CompilerValue> classValue, const STDString& functionName, const V_Array<Share<CompilerValue>>& params);

	// 创建条件判断字节码
public:
	void CreateJmpToBlock(Share<CompilerBlock> block);

	void CreateCompareJmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock);

	Share<CompilerValue> CreateIntCmp(Share<CompilerValue> left, Share<CompilerValue> right);

	Share<CompilerValue> CreateBoolCmp(Share<CompilerValue> value);

public:

	/*void OnCreateClass(Share<CompilerClass> compClass);*/

	/*const HString* GetSymbolTableNameAddress(const HString& className);

	x_uint32 GetSymbolTableNameTypeId(const HString& className);*/

	AdvanceFunctionInfo* GetAdvanceFunctionInfo(HazeValueType advanceType, const STDString& name);

	Share<CompilerEnum> GetBaseModuleEnum(const STDString& name);
	//Share<CompilerEnum> GetBaseModuleEnum(x_uint32 typeId);

	Share<CompilerValue> GetBaseModuleGlobalVariable(const STDString& name);

	Share<CompilerClass> GetBaseModuleClass(const STDString& className);

	bool GetBaseModuleGlobalVariableName(const Share<CompilerValue>& value, HStringView& outName);
	bool GetBaseModuleGlobalVariableId(const Share<CompilerValue>& value, InstructionOpId& outId);

	//void GetRealTemplateTypes(const TemplateDefineTypes& types, V_Array<HazeDefineType>& defineTypes);

	CompilerSymbol* GetCompilerSymbol() { return m_CompilerSymbol.get(); }

	void InsertLineCount(x_int64 lineCount);

	bool IsDebug() const;

	const ParseStage GetParseStage() const { return m_ParseStage; }
	bool IsStage1() const { return m_ParseStage == ParseStage::RegisterSymbol; }
	bool IsStage2() const { return m_ParseStage == ParseStage::ParseAll; }
	bool IsFinishStage() const { return m_ParseStage == ParseStage::Finish; }

	void NextStage();

private:
	HazeVM* m_VM;

	HashMap<HazeValueType, AdvanceClassIndexInfo> m_AdvanceClassIndexInfo;

	V_Array<STDString> m_ModuleNameStack;

	HashMap<STDString, Unique<CompilerModule>> m_CompilerModules;
	HashMap<STDString, CompilerModule*> m_CompilerBaseModules;

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
	//HashMap<HashHString, Pair<Share<CompilerClass>, x_uint32>> m_SymbolTable;

	//BaseBlock
	Share<CompilerBlock> m_InsertBaseBlock;

	Unique<CompilerSymbol> m_CompilerSymbol;

	HashMap<STDString, ModuleParseInfo> m_ModuleParseStates;

	ParseStage m_ParseStage;
	bool m_MarkError;
	bool m_MarkNewCode;

	A_Array<Share<CompilerValue>, 2> m_GlobalRegisters;
};