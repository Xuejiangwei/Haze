#pragma once

#include <memory>
#include <unordered_map>

#include "Haze.h"

class HazeCompilerValue;
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

public:
	static std::shared_ptr<HazeCompilerValue> GetRegister(const HAZE_CHAR* Name);

	static const HAZE_CHAR* GetRegisterName(std::shared_ptr<HazeCompilerValue>& Value);

	bool IsClass(const HAZE_STRING& Name);

	const HAZE_CHAR* GetClassName(const HAZE_STRING& Name);

public:
	//Base block : IP
	void SetInsertBlock(std::shared_ptr<HazeBaseBlock> BB);

	void ClearBlockPoint();
 
	void StoreValue(std::shared_ptr<HazeCompilerValue> Alloca, std::shared_ptr<HazeCompilerValue> Value);

public:		//生成op code
	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineVariable& Variable);

	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& Module, const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> CreateRet(std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateSub(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);
	
	std::shared_ptr<HazeCompilerValue> CreateMul(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);
	
	std::shared_ptr<HazeCompilerValue> CreateDiv(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& Param);

	std::shared_ptr<HazeCompilerValue> CreateNew(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineType& Data);

public:
	std::shared_ptr<HazeCompilerValue> CreateCompare(std::shared_ptr<HazeCompilerValue> ConditionValue);

private:
	void ClearFunctionTemp();

private:
	void GenModuleCodeFile();

private:
	std::vector<HAZE_STRING> Vector_ModuleNameStack;

	std::unordered_map<HAZE_STRING, std::unique_ptr<HazeCompilerModule>> HashMap_CompilerModule;

	//常量
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
