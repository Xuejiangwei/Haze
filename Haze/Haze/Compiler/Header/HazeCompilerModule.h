#pragma once

#include <unordered_map>
#include <fstream>

#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerFunction;

class HazeCompilerModule
{
public:
	friend class HazeCompiler;

	HazeCompilerModule(const HAZE_STRING& ModuleName);
	~HazeCompilerModule();

	void GenCodeFile();

	std::shared_ptr<HazeCompilerFunction> GetCurrFunction();

	std::shared_ptr<HazeCompilerFunction> GetFunction(const HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerFunction> CreateFunction(HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param);

	void FinishFunction();

	std::shared_ptr<HazeCompilerValue> AddGlobalStringVariable(const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> GetGlobalVariable(const HAZE_STRING& Name);

	bool GetGlobalVariableName(std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	HazeValueType FindClass(const HAZE_STRING& ClassName);

public:
	std::shared_ptr<HazeCompilerValue> CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> GenIRCode_BinaryOperater(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, InstructionOpCode IO_Code);

	void GenIRCode_Ret(std::shared_ptr<HazeCompilerValue> Value);

private:
	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> CallFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& Param);

private:
	void GenICode();

private:
	HAZE_OFSTREAM FS_I_Code;

	HAZE_STRING CurrFunction;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerFunction>> Map_Function;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> Vector_Variable; //这个是Symbol table(符号表)

	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerValue>> Map_StringVariable;
};
