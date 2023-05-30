#pragma once

#include <memory>
#include <unordered_map>

#include "Haze.h"

class HazeVM;

class HazeCompilerValue;
class HazeCompilerInitListValue;
class HazeCompilerFunction;
class HazeCompilerModule;

class HazeBaseBlock;

class HazeCompiler
{
public:
	HazeCompiler(HazeVM* VM);
	~HazeCompiler();

	bool InitializeCompiler(const HAZE_STRING& ModuleName);

	HazeCompilerModule* ParseModule(const HAZE_STRING& ModuleName);
	
	void FinishModule();

	HazeCompilerModule* GetModule(const HAZE_STRING& Name);

	const HAZE_STRING* GetModuleName(const HazeCompilerModule* Module) const;

	std::unique_ptr<HazeCompilerModule>& GetCurrModule();

	bool CurrModuleIsStdLib();

	std::shared_ptr<HazeCompilerFunction> GetFunction(const HAZE_STRING& Name);

	const HAZE_STRING& GetCurrModuleName() const { return Vector_ModuleNameStack.back(); }

	void PopCurrModule() { return Vector_ModuleNameStack.pop_back(); }

	void AddImportModuleToCurrModule(HazeCompilerModule* Module);

	bool IsClass(const HAZE_STRING& Name);

	const HAZE_CHAR* GetClassName(const HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerValue> GenConstantValue(HazeValueType Type, const HazeValue& Var);

	std::shared_ptr<HazeCompilerValue> GenStringVariable(HAZE_STRING& String);

	std::shared_ptr<HazeCompilerValue> GetGlobalVariable(const HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerValue> GetLocalVariable(const HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerValue> GetConstantValueInt(int V);

	std::shared_ptr<HazeCompilerValue> GenConstantValueBool(bool IsTrue);

	std::shared_ptr<HazeCompilerValue> GetNullPtr(const HazeDefineType& Type);

	bool IsConstantValueBoolTrue(std::shared_ptr<HazeCompilerValue> V);

	bool IsConstantValueBoolFalse(std::shared_ptr<HazeCompilerValue> V);

public:
	static std::shared_ptr<HazeCompilerValue> GetNewRegister(HazeCompilerModule* Module, const HazeDefineType& Data);

	static std::shared_ptr<HazeCompilerValue> GetTempRegister();

	static std::shared_ptr<HazeCompilerValue> GetRegister(const HAZE_CHAR* Name);

	static const HAZE_CHAR* GetRegisterName(const std::shared_ptr<HazeCompilerValue>& Register);

	static std::shared_ptr<HazeCompilerInitListValue> GetInitializeListValue();

public:
	void SetInsertBlock(std::shared_ptr<HazeBaseBlock> BB);

	std::shared_ptr<HazeBaseBlock> GetInsertBlock() { return InsertBaseBlock; }

	void ClearBlockPoint();

public:
	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineVariable& Variable, 
		std::shared_ptr<HazeCompilerValue> RefValue = nullptr, std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize = {}, std::vector<HazeDefineType>* Vector_Param = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& Module, const HazeDefineVariable& Var, std::shared_ptr<HazeCompilerValue> RefValue = nullptr,
		std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize = {}, std::vector<HazeDefineType>*Vector_Param = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateClassVariable(std::unique_ptr<HazeCompilerModule>& Module, const HazeDefineVariable& Var, std::shared_ptr<HazeCompilerValue> RefValue = nullptr,
		std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize = {}, std::vector<HazeDefineType>* Vector_Param = nullptr);
	
	std::shared_ptr<HazeCompilerValue> CreateLea(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateMov(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateMovToPV(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateMovPV(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateRet(std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateSub(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateMul(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateDiv(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateMod(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitAnd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitOr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitNeg(std::shared_ptr<HazeCompilerValue> Value);
	
	std::shared_ptr<HazeCompilerValue> CreateBitXor(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateShl(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateShr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateNot(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateNeg(std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateInc(std::shared_ptr<HazeCompilerValue> Value, bool IsPreInc);

	std::shared_ptr<HazeCompilerValue> CreateDec(std::shared_ptr<HazeCompilerValue> Value, bool IsPreDec);

	std::shared_ptr<HazeCompilerValue> CreateNew(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineType& Data);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo = nullptr);
	
	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerValue> PointerFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo = nullptr);

public:
	std::shared_ptr<HazeCompilerValue> CreateArrayInit(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> InitList);

	std::shared_ptr<HazeCompilerValue> CreateArrayElement(std::shared_ptr<HazeCompilerValue> Value, std::vector<uint32> Index);

	std::shared_ptr<HazeCompilerValue> CreateArrayElement(std::shared_ptr<HazeCompilerValue> Value, std::vector<std::shared_ptr<HazeCompilerValue>> Index);

public:
	std::shared_ptr<HazeCompilerValue> CreatePointerToValue(std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreatePointerToArray(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> Index = nullptr);

	std::shared_ptr<HazeCompilerValue> CreatePointerToArrayElement(std::shared_ptr<HazeCompilerValue> ElementValue);

	std::shared_ptr<HazeCompilerValue> CreatePointerToPointerArray(std::shared_ptr<HazeCompilerValue> PointerArray, std::shared_ptr<HazeCompilerValue> Index = nullptr);

	std::shared_ptr<HazeCompilerValue> CreatePointerToFunction(std::shared_ptr<HazeCompilerFunction> Function);

public:
	void CreateJmpToBlock(std::shared_ptr<HazeBaseBlock> Block);

	void CreateCompareJmp(HazeCmpType CmpType, std::shared_ptr<HazeBaseBlock> IfJmpBlock, std::shared_ptr<HazeBaseBlock> ElseJmpBlock);

	std::shared_ptr<HazeCompilerValue> CreateIntCmp(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateBoolCmp(std::shared_ptr<HazeCompilerValue> Value);

public:
	void InsertLineCount(int64 LineCount);

	bool IsDebug() const;

private:
	void GenModuleCodeFile();

private:
	HazeVM* VM;

	std::vector<HAZE_STRING> Vector_ModuleNameStack;

	std::unordered_map<HAZE_STRING, std::unique_ptr<HazeCompilerModule>> HashMap_CompilerModule;

	//����
	std::unordered_map<bool, std::shared_ptr<HazeCompilerValue>> HashMap_BoolConstantValue;

	std::unordered_map<hbyte, std::shared_ptr<HazeCompilerValue>> HashMap_ByteConstantValue;
	std::unordered_map<uhbyte, std::shared_ptr<HazeCompilerValue>> HashMap_UnsignedByteConstantValue;
	
	std::unordered_map<hchar, std::shared_ptr<HazeCompilerValue>> HashMap_CharConstantValue;
	
	std::unordered_map<short, std::shared_ptr<HazeCompilerValue>> HashMap_ShortConstantValue;
	std::unordered_map<ushort, std::shared_ptr<HazeCompilerValue>> HashMap_UnsignedShortConstantValue;
	
	std::unordered_map<int, std::shared_ptr<HazeCompilerValue>> HashMap_IntConstantValue;
	std::unordered_map<uint32, std::shared_ptr<HazeCompilerValue>> HashMap_UnsignedIntConstantValue;
	
	std::unordered_map<int64, std::shared_ptr<HazeCompilerValue>> HashMap_LongConstantValue;
	std::unordered_map<uint64, std::shared_ptr<HazeCompilerValue>> HashMap_UnsignedLongConstantValue;
	
	std::unordered_map<float, std::shared_ptr<HazeCompilerValue>> HashMap_FloatConstantValue;
	std::unordered_map<double, std::shared_ptr<HazeCompilerValue>> HashMap_DobuleConstantValue;
	
	//BaseBlock
	std::shared_ptr<HazeBaseBlock> InsertBaseBlock;
};
