#pragma once

#include <memory>
#include <unordered_map>

#include "HazeHeader.h"

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

	bool InitializeCompiler(const HAZE_STRING& moduleName, const HAZE_STRING& path);

	void FinishParse();

	HazeCompilerModule* ParseBaseModule(const HAZE_CHAR* moduleName, const HAZE_CHAR* moduleCode);

	HazeCompilerModule* ParseModule(const HAZE_STRING& modulePath);

	void FinishModule();

	HazeCompilerModule* GetModule(const HAZE_STRING& name);

	const HAZE_STRING* GetModuleName(const HazeCompilerModule* compilerModule) const;

	std::unique_ptr<HazeCompilerModule>& GetCurrModule();

	bool CurrModuleIsStdLib();

	std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> GetFunction(const HAZE_STRING& name);

	const HAZE_STRING& GetCurrModuleName() const { return m_ModuleNameStack.back(); }

	void PopCurrModule() { return m_ModuleNameStack.pop_back(); }

	void AddImportModuleToCurrModule(HazeCompilerModule* compilerModule);

	bool IsClass(const HAZE_STRING& name);

	const HAZE_CHAR* GetClassName(const HAZE_STRING& name);

	bool IsTemplateClass(const HAZE_STRING& name);

	void MarkParseTemplate(bool begin, const HAZE_STRING* moduleName = nullptr);

	std::shared_ptr<HazeCompilerValue> GenConstantValue(HazeValueType type, const HazeValue& var);

	std::shared_ptr<HazeCompilerValue> GenStringVariable(HAZE_STRING& str);

	std::shared_ptr<HazeCompilerValue> GetGlobalVariable(const HAZE_STRING& name);

	std::shared_ptr<HazeCompilerValue> GetLocalVariable(const HAZE_STRING& name);

	std::shared_ptr<HazeCompilerValue> GetConstantValueInt(int v);

	std::shared_ptr<HazeCompilerValue> GetConstantValueUint64(uint64 v);

	std::shared_ptr<HazeCompilerValue> GenConstantValueBool(bool isTrue);

	std::shared_ptr<HazeCompilerValue> GetNullPtr(const HazeDefineType& type);

	bool IsConstantValueBoolTrue(std::shared_ptr<HazeCompilerValue> v);

	bool IsConstantValueBoolFalse(std::shared_ptr<HazeCompilerValue> v);

public:
	static std::shared_ptr<HazeCompilerValue> GetNewRegister(HazeCompilerModule* compilerModule, const HazeDefineType& data);

	static std::shared_ptr<HazeCompilerValue> GetTempRegister();

	static std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>> GetUseTempRegister();

	static void ClearTempRegister(const std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>>& useTempRegisters);

	static void ResetTempRegister(const std::unordered_map<const HAZE_CHAR*, std::shared_ptr<HazeCompilerValue>>& useTempRegisters);

	static std::shared_ptr<HazeCompilerValue> GetRegister(const HAZE_CHAR* name);

	static const HAZE_CHAR* GetRegisterName(const std::shared_ptr<HazeCompilerValue>& compilerRegister);

	static std::shared_ptr<HazeCompilerInitListValue> GetInitializeListValue();

	static constexpr int GetMaxPointerLevel() { return 2; }

public:
	void SetInsertBlock(std::shared_ptr<HazeBaseBlock> block);

	std::shared_ptr<HazeBaseBlock> GetInsertBlock() { return m_InsertBaseBlock; }

	void ClearBlockPoint();

public:
	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineVariable& Variable, int Line,
		std::shared_ptr<HazeCompilerValue> RefValue = nullptr, std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize = {}, std::vector<HazeDefineType>* Params = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& m_Module, const HazeDefineVariable& Var, std::shared_ptr<HazeCompilerValue> RefValue = nullptr,
		std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize = {}, std::vector<HazeDefineType>* Params = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateClassVariable(std::unique_ptr<HazeCompilerModule>& m_Module, const HazeDefineVariable& Var, std::shared_ptr<HazeCompilerValue> RefValue = nullptr,
		std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize = {}, std::vector<HazeDefineType>* Params = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateLea(std::shared_ptr<HazeCompilerValue> allocaValue, std::shared_ptr<HazeCompilerValue> value);

	std::shared_ptr<HazeCompilerValue> CreateMov(std::shared_ptr<HazeCompilerValue> allocaValue, std::shared_ptr<HazeCompilerValue> value, bool storeValue = true);

	std::shared_ptr<HazeCompilerValue> CreateMovToPV(std::shared_ptr<HazeCompilerValue> allocaValue, std::shared_ptr<HazeCompilerValue> value);

	std::shared_ptr<HazeCompilerValue> CreateMovPV(std::shared_ptr<HazeCompilerValue> allocaValue, std::shared_ptr<HazeCompilerValue> value);

	std::shared_ptr<HazeCompilerValue> CreateRet(std::shared_ptr<HazeCompilerValue> value);

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

	std::shared_ptr<HazeCompilerValue> CreateNew(std::shared_ptr<HazeCompilerFunction> function, const HazeDefineType& data, std::shared_ptr<HazeCompilerValue> countValue);

	std::shared_ptr<HazeCompilerValue> CreateCast(const HazeDefineType& type, std::shared_ptr<HazeCompilerValue> value);

	std::shared_ptr<HazeCompilerValue> CreateCVT(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> function, std::vector<std::shared_ptr<HazeCompilerValue>>& param, std::shared_ptr<HazeCompilerValue> thisPointerTo = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerValue> pointerFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& param, std::shared_ptr<HazeCompilerValue> thisPointerTo = nullptr);

public:
	std::shared_ptr<HazeCompilerValue> CreateArrayInit(std::shared_ptr<HazeCompilerValue> arrValue, std::shared_ptr<HazeCompilerValue> initList);

	std::shared_ptr<HazeCompilerValue> CreateArrayElement(std::shared_ptr<HazeCompilerValue> value, std::vector<uint32> index);

	std::shared_ptr<HazeCompilerValue> CreateArrayElement(std::shared_ptr<HazeCompilerValue> value, std::vector<std::shared_ptr<HazeCompilerValue>> indices);

	std::shared_ptr<HazeCompilerValue> CreateGetArrayLength(std::shared_ptr<HazeCompilerValue> value);

public:
	std::shared_ptr<HazeCompilerValue> CreatePointerToValue(std::shared_ptr<HazeCompilerValue> value);

	std::shared_ptr<HazeCompilerValue> CreatePointerToArray(std::shared_ptr<HazeCompilerValue> arrValue, std::shared_ptr<HazeCompilerValue> index = nullptr);

	std::shared_ptr<HazeCompilerValue> CreatePointerToArrayElement(std::shared_ptr<HazeCompilerValue> elementValue);

	std::shared_ptr<HazeCompilerValue> CreatePointerToPointerArray(std::shared_ptr<HazeCompilerValue> pointerArray, std::shared_ptr<HazeCompilerValue> index = nullptr);

	std::shared_ptr<HazeCompilerValue> CreatePointerToFunction(std::shared_ptr<HazeCompilerFunction> function);

public:
	void CreateJmpToBlock(std::shared_ptr<HazeBaseBlock> block);

	void CreateCompareJmp(HazeCmpType cmpType, std::shared_ptr<HazeBaseBlock> ifJmpBlock, std::shared_ptr<HazeBaseBlock> elseJmpBlock);

	std::shared_ptr<HazeCompilerValue> CreateIntCmp(std::shared_ptr<HazeCompilerValue> left, std::shared_ptr<HazeCompilerValue> right);

	std::shared_ptr<HazeCompilerValue> CreateBoolCmp(std::shared_ptr<HazeCompilerValue> value);

	void ReplaceConstantValueByStrongerType(std::shared_ptr<HazeCompilerValue>& left, std::shared_ptr<HazeCompilerValue>& right);

public:
	std::shared_ptr<HazeCompilerEnum> GetBaseModuleEnum(const HAZE_STRING& name);

	std::shared_ptr<HazeCompilerValue> GetBaseModuleGlobalVariable(const HAZE_STRING& name);

	std::shared_ptr<HazeCompilerClass> GetBaseModuleClass(const HAZE_STRING& className);

	bool GetBaseModuleGlobalVariableName(const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName);

	void InsertLineCount(int64 lineCount);

	bool IsDebug() const;

private:
	HazeVM* m_VM;

	std::vector<HAZE_STRING> m_ModuleNameStack;

	std::unordered_map<HAZE_STRING, std::unique_ptr<HazeCompilerModule>> m_CompilerModules;
	std::unordered_map<HAZE_STRING, HazeCompilerModule*> m_CompilerBaseModules;

	//³£Á¿
	std::unordered_map<bool, std::shared_ptr<HazeCompilerValue>> m_BoolConstantValues;

	std::unordered_map<hbyte, std::shared_ptr<HazeCompilerValue>> m_ByteConstantValues;
	std::unordered_map<uhbyte, std::shared_ptr<HazeCompilerValue>> m_UnsignedByteConstantValues;

	std::unordered_map<hchar, std::shared_ptr<HazeCompilerValue>> m_CharConstantValues;

	std::unordered_map<short, std::shared_ptr<HazeCompilerValue>> m_ShortConstantValues;
	std::unordered_map<ushort, std::shared_ptr<HazeCompilerValue>> m_UnsignedShortConstantValues;

	std::unordered_map<int, std::shared_ptr<HazeCompilerValue>> m_IntConstantValues;
	std::unordered_map<uint32, std::shared_ptr<HazeCompilerValue>> m_UnsignedIntConstantValues;

	std::unordered_map<int64, std::shared_ptr<HazeCompilerValue>> m_LongConstantValues;
	std::unordered_map<uint64, std::shared_ptr<HazeCompilerValue>> m_UnsignedLongConstantValues;

	std::unordered_map<float, std::shared_ptr<HazeCompilerValue>> m_FloatConstantValues;
	std::unordered_map<double, std::shared_ptr<HazeCompilerValue>> m_DobuleConstantValues;

	//BaseBlock
	std::shared_ptr<HazeBaseBlock> m_InsertBaseBlock;
};
