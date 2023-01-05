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

	std::shared_ptr<HazeCompilerFunction> CreateFunction(HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param);

	void FinishFunction();

	std::shared_ptr<HazeCompilerValue> AddGlobalStringVariable(const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> GetGlobalVariable(const HAZE_STRING& Name);

	HazeValueType FindClass(const HAZE_STRING& ClassName);

public:
	template<typename T>
	void Stream(T Value)
	{
		FS_Ass << Value;
	}

	template<typename T>
	void Push(T Value)
	{
		FS_Ass << "Push " << Value << std::endl;
		StackTop++;
	}

	void Pop()
	{
		StackTop--;
	}

	void GenASS_Label(HAZE_STRING& Label);
	
	void GenASS_Add(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

private:
	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param);

private:
	void GenASS_FileHeader();
	void GenASS_Instruction();
	void GenASS_GlobalData();

private:
	HAZE_OFSTREAM FS_Ass;
	HAZE_OFSTREAM FS_OpCode;

	int StackTop;

	HAZE_STRING CurrFunction;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerFunction>> MapGlobalFunction;

	unsigned int GlobalVariableSize;
	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> VectorGlobalVariable;

	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerValue>> MapGlobalStringVariable;

};
