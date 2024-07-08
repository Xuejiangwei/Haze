#pragma once

class HazeVM;
class HazeCompilerValue;
class HazeCompilerInitListValue;
class HazeCompilerFunction;
class HazeCompilerEnum;
class HazeCompilerClass;
class HazeCompilerModule;
class HazeBaseBlock;

class HazeCompiler
{
public:
	HazeCompiler(HazeVM* vm);

	~HazeCompiler();

	bool InitializeCompiler(const HString& moduleName, const HString& path);

	void FinishParse();

	HazeCompilerModule* ParseBaseModule(const HChar* moduleName, const HChar* moduleCode);

	HazeCompilerModule* ParseModule(const HString& modulePath);

	void FinishModule();

	HazeCompilerModule* GetModule(const HString& name);

	const HString* GetModuleName(const HazeCompilerModule* compilerModule) const;

	Unique<HazeCompilerModule>& GetCurrModule();

	bool CurrModuleIsStdLib();

	Pair<Share<HazeCompilerFunction>, Share<HazeCompilerValue>> GetFunction(const HString& name);

	const HString& GetCurrModuleName() const { return m_ModuleNameStack.back(); }

	void PopCurrModule() { return m_ModuleNameStack.pop_back(); }

	void AddImportModuleToCurrModule(HazeCompilerModule* compilerModule);

	bool IsClass(const HString& name);

	const HChar* GetClassName(const HString& name);

	bool IsTemplateClass(const HString& name);

	void MarkParseTemplate(bool begin, const HString* moduleName = nullptr);

	Share<HazeCompilerValue> GenConstantValue(HazeValueType type, const HazeValue& var);

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

	static constexpr int GetMaxPointerLevel() { return 2; }

public:
	void SetInsertBlock(Share<HazeBaseBlock> block);

	Share<HazeBaseBlock> GetInsertBlock() { return m_InsertBaseBlock; }

	void ClearBlockPoint();

public:
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

	Share<HazeCompilerValue> CreateNew(Share<HazeCompilerFunction> function, const HazeDefineType& data, Share<HazeCompilerValue> countValue);

	Share<HazeCompilerValue> CreateCast(const HazeDefineType& type, Share<HazeCompilerValue> value);

	Share<HazeCompilerValue> CreateCVT(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right);

	Share<HazeCompilerValue> CreateFunctionCall(Share<HazeCompilerFunction> function, V_Array<Share<HazeCompilerValue>>& param, Share<HazeCompilerValue> thisPointerTo = nullptr);

	Share<HazeCompilerValue> CreateFunctionCall(Share<HazeCompilerValue> pointerFunction, V_Array<Share<HazeCompilerValue>>& param, Share<HazeCompilerValue> thisPointerTo = nullptr);

public:
	Share<HazeCompilerValue> CreateArrayInit(Share<HazeCompilerValue> arrValue, Share<HazeCompilerValue> initList);

	Share<HazeCompilerValue> CreateArrayElement(Share<HazeCompilerValue> value, V_Array<uint32> index);

	Share<HazeCompilerValue> CreateArrayElement(Share<HazeCompilerValue> value, V_Array<Share<HazeCompilerValue>> indices);

	Share<HazeCompilerValue> CreateGetArrayLength(Share<HazeCompilerValue> value);

public:
	Share<HazeCompilerValue> CreatePointerToValue(Share<HazeCompilerValue> value);

	Share<HazeCompilerValue> CreatePointerToArray(Share<HazeCompilerValue> arrValue, Share<HazeCompilerValue> index = nullptr);

	Share<HazeCompilerValue> CreatePointerToArrayElement(Share<HazeCompilerValue> elementValue);

	Share<HazeCompilerValue> CreatePointerToPointerArray(Share<HazeCompilerValue> pointerArray, Share<HazeCompilerValue> index = nullptr);

	Share<HazeCompilerValue> CreatePointerToFunction(Share<HazeCompilerFunction> function);

public:
	void CreateJmpToBlock(Share<HazeBaseBlock> block);

	void CreateCompareJmp(HazeCmpType cmpType, Share<HazeBaseBlock> ifJmpBlock, Share<HazeBaseBlock> elseJmpBlock);

	Share<HazeCompilerValue> CreateIntCmp(Share<HazeCompilerValue> left, Share<HazeCompilerValue> right);

	Share<HazeCompilerValue> CreateBoolCmp(Share<HazeCompilerValue> value);

	void ReplaceConstantValueByStrongerType(Share<HazeCompilerValue>& left, Share<HazeCompilerValue>& right);

public:
	Share<HazeCompilerEnum> GetBaseModuleEnum(const HString& name);

	Share<HazeCompilerValue> GetBaseModuleGlobalVariable(const HString& name);

	Share<HazeCompilerClass> GetBaseModuleClass(const HString& className);

	bool GetBaseModuleGlobalVariableName(const Share<HazeCompilerValue>& value, HString& outName);

	void InsertLineCount(int64 lineCount);

	bool IsDebug() const;

private:
	HazeVM* m_VM;

	V_Array<HString> m_ModuleNameStack;

	HashMap<HString, Unique<HazeCompilerModule>> m_CompilerModules;
	HashMap<HString, HazeCompilerModule*> m_CompilerBaseModules;

	//³£Á¿
	HashMap<bool, Share<HazeCompilerValue>> m_BoolConstantValues;

	HashMap<HByte, Share<HazeCompilerValue>> m_ByteConstantValues;
	HashMap<uhbyte, Share<HazeCompilerValue>> m_UnsignedByteConstantValues;

	HashMap<HChar, Share<HazeCompilerValue>> m_CharConstantValues;

	HashMap<short, Share<HazeCompilerValue>> m_ShortConstantValues;
	HashMap<ushort, Share<HazeCompilerValue>> m_UnsignedShortConstantValues;

	HashMap<int, Share<HazeCompilerValue>> m_IntConstantValues;
	HashMap<uint32, Share<HazeCompilerValue>> m_UnsignedIntConstantValues;

	HashMap<int64, Share<HazeCompilerValue>> m_LongConstantValues;
	HashMap<uint64, Share<HazeCompilerValue>> m_UnsignedLongConstantValues;

	HashMap<float, Share<HazeCompilerValue>> m_FloatConstantValues;
	HashMap<double, Share<HazeCompilerValue>> m_DobuleConstantValues;

	//BaseBlock
	Share<HazeBaseBlock> m_InsertBaseBlock;
};
