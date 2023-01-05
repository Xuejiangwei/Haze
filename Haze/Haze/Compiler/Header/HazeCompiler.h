#pragma once

#include <memory>
#include <unordered_map>

#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerFunction;
class HazeCompilerModule;

class HazeCompiler
{
public:
	HazeCompiler();
	~HazeCompiler();

	bool InitializeCompiler(const HAZE_STRING& ModuleName);

	void FinishModule();

	std::unique_ptr<HazeCompilerModule>& GetCurrModule();

	std::shared_ptr<HazeCompilerValue> GenConstantValue(const HazeValue& Var);

	std::shared_ptr<HazeCompilerValue> GetGlobalVariable(const HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerValue> GetLocalVariable(const HAZE_STRING& Name);

public:		//����op code
	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(std::shared_ptr<HazeCompilerFunction> Function, const HazeDefineVariable& Variable);

	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(std::unique_ptr<HazeCompilerModule>& Module, const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param);

private:
	void GenModuleCodeFile();

private:
	HAZE_STRING CurrModule;

	std::unordered_map<HAZE_STRING, std::unique_ptr<HazeCompilerModule>> MapModules;

	//����
	std::unordered_map<bool, std::shared_ptr<HazeCompilerValue>> MapBoolConstantValue;
	std::unordered_map<char, std::shared_ptr<HazeCompilerValue>> MapIntConstantValue;
	std::unordered_map<long long, std::shared_ptr<HazeCompilerValue>> MapLongConstantValue;
	std::unordered_map<unsigned int, std::shared_ptr<HazeCompilerValue>> MapUnsignedIntConstantValue;
	std::unordered_map<unsigned long long, std::shared_ptr<HazeCompilerValue>> MapUnsignedLongConstantValue;
	std::unordered_map<float, std::shared_ptr<HazeCompilerValue>> MapFloatConstantValue;
	std::unordered_map<double, std::shared_ptr<HazeCompilerValue>> MapDobuleConstantValue;
	std::unordered_map<HazeValueType, std::shared_ptr<HazeCompilerValue>> MapDefineConstantValue;
};
