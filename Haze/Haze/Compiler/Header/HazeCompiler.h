#pragma once

class HazeVM;
class HazeCompilerValue;
class HazeCompilerInitListValue;
class HazeCompilerFunction;
class HazeCompilerEnum;
class HazeCompilerClass;
class HazeCompilerModule;
class HazeBaseBlock;

struct AdvanceFunctionInfo
{
	void(*Func)(class HazeStack*);
	HazeDefineType Type;
	V_Array<HazeDefineType> Params;
};

struct AdvanceClassInfo
{
	HashMap<HString, AdvanceFunctionInfo> Functions;
};

class HazeCompiler
{
public:
	HazeCompiler(HazeVM* vm);

	~HazeCompiler();

	void RegisterAdvanceClassInfo(HazeValueType type, AdvanceClassInfo& info);

	bool InitializeCompiler(const HString& moduleName, const HString& path);

	void FinishParse();

	HazeCompilerModule* ParseBaseModule(const HChar* moduleName, const HChar* moduleCode);

	HazeCompilerModule* ParseModule(const HString& modulePath);

	void FinishModule();

	HazeCompilerModule* GetModule(const HString& name);

	const HString* GetModuleName(const HazeCompilerModule* compilerModule) const;

	const HString* GetModuleTableClassName(const HString& name);
	const HString* GetModuleTableEnumName(const HString& name);

	Unique<HazeCompilerModule>& GetCurrModule();

	bool CurrModuleIsStdLib();

	Pair<Share<HazeCompilerFunction>, Share<HazeCompilerValue>> GetFunction(const HString& name);

	const HString& GetCurrModuleName() const { return m_ModuleNameStack.back(); }

	void PopCurrModule() { return m_ModuleNameStack.pop_back(); }

	void AddImportModuleToCurrModule(HazeCompilerModule* compilerModule);

	bool IsClass(const HString& name);

	bool IsEnum(const HString& name);

	bool IsTemplateClass(const HString& name);

	void MarkParseTemplate(bool begin, const HString* moduleName = nullptr);

	Share<HazeCompilerValue> GenConstantValue(HazeValueType type, const HazeValue& var, HazeValueType* varType = nullptr);

	Share<HazeCompilerValue> GenStringVariable(HString& str);

	Share<HazeCompilerValue> GetGlobalVariable(const HString& name);

	Share<HazeCompilerValue> GetLocalVariable(const HString& name);

	Share<HazeCompilerValue> GetEnumVariable(const HString& enumName, const HString& name);

	Share<HazeCompilerValue> GetConstantValueInt(int v);

	Share<HazeCompilerValue> GetConstantValueUint64(uint64 v);

	Share<HazeCompilerValue> GenConstantValueBool(bool isTrue);

	Share<HazeCompilerValue> GetNullPtr(const HazeDefineType& type);

	bool IsConstantValueBoolTrue(Share<HazeCompilerValue> v);

	bool IsConstantValueBoolFalse(Share<HazeCompilerValue> v);

public:
	static Share<HazeCompilerValue> GetNewRegister(HazeCompilerModule* compilerModule, const HazeDefineType& data);

	static Share<HazeCompilerValue> GetTempRegister();

	static HashMap<const HChar*, Share<HazeCompilerValue>> GetUseTempRegister();

	static void ClearTempRegister(const HashMap<const HChar*, Share<HazeCompilerValue>>& useTempRegisters);

	static void ResetTempRegister(const HashMap<const HChar*, Share<HazeCompilerValue>>& useTempRegisters);

	static Share<HazeCompilerValue> GetRegister(const HChar* name);

	static const HChar* GetRegisterName(const Share<HazeCompilerValue>& compilerRegister);

	static Share<HazeCompilerInitListValue> GetInitializeListValue();

public:
	void SetInsertBlock(Share<HazeBaseBlock> block);

	Share<HazeBaseBlock> GetInsertBlock() { return m_InsertBaseBlock; }

	void ClearBlockPoint();

public:
	Share<HazeCompilerValue> CreateVariableBySection(HazeSectionSignal section, Unique<HazeCompilerModule>& mod, Share<HazeCompilerFunction> func,
		const HazeDefineVariable& var, int line, Share<HazeCompilerValue> refValue = nullptr, V_Array<Share<HazeCompilerValue>> arraySize = {},
		V_Array<HazeDefineType>* params = nullptr);

	Share<HazeCompilerValue> CreateLocalVariable(Share<HazeCompilerFunction> Function, const HazeDefineVariable& Variable, int Line,
		Share<HazeCompilerValue> RefValue = nullptr, V_Array<Share<HazeCompilerValue>> m_ArraySize = {}, V_Array<HazeDefineType>* Params = nullptr);

	Share<HazeCompilerValue> CreateGlobalVariable(Unique<HazeCompilerModule>& m_Module, const HazeDefineVariable& Var, Share<HazeCompilerValue> RefValue = nullptr,
		V_Array<Share<HazeCompilerValue>> m_ArraySize = {}, V_Array<HazeDefineType>* Params = nullptr);

	Share<HazeCompilerValue> CreateClassVariable(Unique<HazeCompilerModule>& m_Module, const HazeDefineVariable& Var, Share<HazeCompilerValue> RefValue = nullptr,
		V_Array<Share<HazeCompilerValue>> m_ArraySize = {}, V_Array<HazeDefineType>* Params = nullptr);

	Share<HazeCompilerValue> CreateLea(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value);

	Share<HazeCompilerValue> CreateMov(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value, bool storeValue = true);

	Share<HazeCompilerValue> CreateMovToPV(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value);

	Share<HazeCompilerValue> CreateMovPV(Share<HazeCompilerValue> allocaValue, Share<HazeCompilerValue> value);

	Share<HazeCompilerValue> CreateRet(Share<HazeCompilerValue> value);

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

	Share<HazeCompilerValue> CreateNew(Share<HazeCompilerFunction> function, const HazeDefineType& data, V_Array<Share<HazeCompilerValue>>* countValue);
	
	Share<HazeCompilerValue> CreateCast(const HazeDefineType& type, Share<HazeCompilerValue> value);

	Share<HazeCompilerValue> CreateCVT(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right);

	Share<HazeCompilerValue> CreateFunctionCall(Share<HazeCompilerFunction> function, V_Array<Share<HazeCompilerValue>>& param, Share<HazeCompilerValue> thisPointerTo = nullptr);

	Share<HazeCompilerValue> CreateFunctionCall(Share<HazeCompilerValue> pointerFunction, V_Array<Share<HazeCompilerValue>>& param, Share<HazeCompilerValue> thisPointerTo = nullptr);

	Share<HazeCompilerValue> CreateAdvanceTypeFunctionCall(HazeValueType advanceType, const HString& functionName,
		V_Array<Share<HazeCompilerValue>>& param, Share<HazeCompilerValue> thisPointerTo);

public:
	Share<HazeCompilerValue> CreateArrayInit(Share<HazeCompilerValue> arrValue, Share<HazeCompilerValue> initList);

	Share<HazeCompilerValue> CreateArrayElement(Share<HazeCompilerValue> value, V_Array<uint32> index);

	Share<HazeCompilerValue> CreateArrayElement(Share<HazeCompilerValue> value, V_Array<Share<HazeCompilerValue>> indices);

public:
	Share<HazeCompilerValue> CreatePointerToValue(Share<HazeCompilerValue> value);

	//Share<HazeCompilerValue> CreatePointerToArray(Share<HazeCompilerValue> arrValue, Share<HazeCompilerValue> index = nullptr);

	//Share<HazeCompilerValue> CreatePointerToArrayElement(Share<HazeCompilerValue> elementValue);

	//Share<HazeCompilerValue> CreatePointerToPointerArray(Share<HazeCompilerValue> pointerArray, Share<HazeCompilerValue> index = nullptr);

	Share<HazeCompilerValue> CreatePointerToFunction(Share<HazeCompilerFunction> function, Share<HazeCompilerValue> pointer);

public:
	void CreateJmpToBlock(Share<HazeBaseBlock> block);

	void CreateCompareJmp(HazeCmpType cmpType, Share<HazeBaseBlock> ifJmpBlock, Share<HazeBaseBlock> elseJmpBlock);

	Share<HazeCompilerValue> CreateIntCmp(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right);

	Share<HazeCompilerValue> CreateBoolCmp(Share<HazeCompilerValue> value);

	void ReplaceConstantValueByStrongerType(Share<HazeCompilerValue>& left, Share<HazeCompilerValue>& right);

public:
	void RegisterClassToSymbolTable(const HString& className);

	const HString* GetSymbolTableNameAddress(const HString& className);

	Share<HazeCompilerEnum> GetBaseModuleEnum(const HString& name);

	Share<HazeCompilerValue> GetBaseModuleGlobalVariable(const HString& name);

	Share<HazeCompilerClass> GetBaseModuleClass(const HString& className);

	bool GetBaseModuleGlobalVariableName(const Share<HazeCompilerValue>& value, HString& outName);

	void GetRealTemplateTypes(const TemplateDefineTypes& types, V_Array<HazeDefineType>& defineTypes);

	void InsertLineCount(int64 lineCount);

	bool IsDebug() const;

private:
	HazeVM* m_VM;

	HashMap<HazeValueType, AdvanceClassInfo> m_AdvanceClassInfo;

	V_Array<HString> m_ModuleNameStack;

	HashMap<HString, Unique<HazeCompilerModule>> m_CompilerModules;
	HashMap<HString, HazeCompilerModule*> m_CompilerBaseModules;

	//³£Á¿
	HashMap<bool, Share<HazeCompilerValue>> m_BoolConstantValues;

	HashMap<int8, Share<HazeCompilerValue>> m_Int8_ConstantValues;
	HashMap<uint8, Share<HazeCompilerValue>> m_UInt8_ConstantValues;
	HashMap<int16, Share<HazeCompilerValue>> m_Int16_ConstantValues;
	HashMap<uint16, Share<HazeCompilerValue>> m_UInt16_ConstantValues;
	HashMap<int32, Share<HazeCompilerValue>> m_Int32_ConstantValues;
	HashMap<uint32, Share<HazeCompilerValue>> m_UInt32_ConstantValues;
	HashMap<int64, Share<HazeCompilerValue>> m_Int64_ConstantValues;
	HashMap<uint64, Share<HazeCompilerValue>> m_UInt64_ConstantValues;

	HashMap<float32, Share<HazeCompilerValue>> m_Float32_ConstantValues;
	HashMap<float64, Share<HazeCompilerValue>> m_Float64_ConstantValues;

	HashMap<HString, Share<HazeCompilerClass>> m_SymbolTable;

	//BaseBlock
	Share<HazeBaseBlock> m_InsertBaseBlock;
};
