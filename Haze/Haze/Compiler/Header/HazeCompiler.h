#pragma once

#include <memory>
#include <unordered_map>

#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerInitListValue;
class HazeCompilerFunction;
class HazeCompilerModule;

class HazeBaseBlock;

class HazeCompiler
{
public:
	HazeCompiler();
	~HazeCompiler();

	bool InitializeCompiler(const HAZE_STRING& ModuleName);

	void FinishModule();

	std::unique_ptr<HazeCompilerModule>& GetCurrModule();

	bool CurrModuleIsStdLib();

	std::shared_ptr<HazeCompilerFunction> GetFunction(const HAZE_STRING& Name);

	const HAZE_STRING& GetCurrModuleName() const { return Vector_ModuleNameStack.back(); }

	void PopCurrModule() { return Vector_ModuleNameStack.pop_back(); }

	HAZE_STRING GetCurrModuleOpFile() const;

	std::shared_ptr<HazeCompilerValue> GenConstantValue(const HazeValue& Var);

	std::shared_ptr<HazeCompilerValue> GenStringVariable(HAZE_STRING& String);

	std::shared_ptr<HazeCompilerValue> GetGlobalVariable(const HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerValue> GetLocalVariable(const HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerValue> GetConstantValueInt_1();

public:
	static std::shared_ptr<HazeCompilerValue> GetNewRegister(HazeCompilerModule* Module, const HazeDefineType& Data);

	static std::shared_ptr<HazeCompilerValue> GetRegister(const HAZE_CHAR* Name);

	static const HAZE_CHAR* GetRegisterName(const std::shared_ptr<HazeCompilerValue>& Value);

	static std::shared_ptr<HazeCompilerInitListValue> GetInitializeListValue();

	bool IsClass(const HAZE_STRING& Name);

	const HAZE_CHAR* GetClassName(const HAZE_STRING& Name);

public:
	void SetInsertBlock(std::shared_ptr<HazeBaseBlock> BB);

	std::shared_ptr<HazeBaseBlock> GetInsertBlock() { return InsertBaseBlock; }

	void ClearBlockPoint();
 
public:		//����op code
	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineVariable& Variable, std::shared_ptr<HazeCompilerValue> ArraySize = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& Module, const HazeDefineVariable& Var, std::shared_ptr<HazeCompilerValue> ArraySize = nullptr);
	
	std::shared_ptr<HazeCompilerValue> CreateMov(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateRet(std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateSub(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateMul(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateDiv(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateMod(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitAnd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitOr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitXor(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateShl(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateShr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateAnd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateOr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateNot(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateInc(std::shared_ptr<HazeCompilerValue> Value, bool IsPreInc);

	std::shared_ptr<HazeCompilerValue> CreateDec(std::shared_ptr<HazeCompilerValue> Value, bool IsPreDec);

	std::shared_ptr<HazeCompilerValue> CreateOperatorAssign(HazeOperatorAssign Type, std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateNew(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineType& Data);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo = nullptr);

public:
	std::shared_ptr<HazeCompilerValue> CreateArrayInit(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> InitList);

	std::shared_ptr<HazeCompilerValue> CreateArrayElement(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> Index);

public:
	void CreateJmpFromBlock(std::shared_ptr<HazeBaseBlock> FromBlock, std::shared_ptr<HazeBaseBlock> ToBlock, bool IsJmpL = false);

	void CreateJmpToBlock(std::shared_ptr<HazeBaseBlock> Block, bool IsJmpL = false);

	void CreateCompareJmp(HazeCmpType CmpType, std::shared_ptr<HazeBaseBlock> IfJmpBlock, std::shared_ptr<HazeBaseBlock> ElseJmpBlock, bool IfNullJmpOut = false, bool ElseNullJmpOut = false);
	
	std::shared_ptr<HazeCompilerValue> CreateIntCmp(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

private:
	void GenModuleCodeFile();

private:
	std::vector<HAZE_STRING> Vector_ModuleNameStack;

	std::unordered_map<HAZE_STRING, std::unique_ptr<HazeCompilerModule>> HashMap_CompilerModule;

	//����
	std::unordered_map<bool, std::shared_ptr<HazeCompilerValue>> HashMap_BoolConstantValue;
	std::unordered_map<int, std::shared_ptr<HazeCompilerValue>> HashMap_IntConstantValue;
	std::unordered_map<long long, std::shared_ptr<HazeCompilerValue>> HashMap_LongConstantValue;
	std::unordered_map<unsigned int, std::shared_ptr<HazeCompilerValue>> HashMap_UnsignedIntConstantValue;
	std::unordered_map<unsigned long long, std::shared_ptr<HazeCompilerValue>> HashMap_UnsignedLongConstantValue;
	std::unordered_map<float, std::shared_ptr<HazeCompilerValue>> HashMap_FloatConstantValue;
	std::unordered_map<double, std::shared_ptr<HazeCompilerValue>> HashMap_DobuleConstantValue;
	std::unordered_map<HazeValueType, std::shared_ptr<HazeCompilerValue>> HashMap_DefineConstantValue;

	//BaseBlock
	std::shared_ptr<HazeBaseBlock> InsertBaseBlock;
};
