#pragma once

#include <unordered_map>
#include <fstream>

#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerFunction;
class HazeCompilerClass;

class HazeCompilerModule
{
public:
	friend class HazeCompiler;

	HazeCompilerModule(const HAZE_STRING& ModuleName);
	~HazeCompilerModule();

	void MarkStandardLibrary();
	
	void GenCodeFile();

	std::shared_ptr<HazeCompilerClass> CreateClass(const HAZE_STRING& Name, const HazeClassData& ClassData);

	std::shared_ptr<HazeCompilerFunction> GetCurrFunction();

	std::shared_ptr<HazeCompilerFunction> CreateFunction(HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param);

	void FinishFunction();

	std::shared_ptr<HazeCompilerValue> GetGlobalStringVariable(const HAZE_STRING& String);

	std::shared_ptr<HazeCompilerValue> GetGlobalVariable(const HAZE_STRING& Name);

	bool GetGlobalVariableName(std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	std::shared_ptr<HazeCompilerClass> FindClass(const HAZE_STRING& ClassName);

public:
	std::shared_ptr<HazeCompilerValue> CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> GenIRCode_BinaryOperater(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, InstructionOpCode IO_Code);

	void GenIRCode_Ret(std::shared_ptr<HazeCompilerValue> Value);

private:
	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> CallFunction, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& Param);

	std::shared_ptr<HazeCompilerFunction> GetFunction(const HAZE_STRING& Name);

	bool IsStandardLibrary() { return IsStdLib; }
private:
	void GenICode();

private:
	bool IsStdLib;

	HAZE_OFSTREAM FS_I_Code;

	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerClass>> HashMap_Class;

	HAZE_STRING CurrFunction;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerFunction>> HashMap_Function;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> Vector_Variable; //这个是Symbol table(符号表)

	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerValue>> HashMap_StringTable;
	std::unordered_map<int, const HAZE_STRING*> HashMap_StringMapping;
};
