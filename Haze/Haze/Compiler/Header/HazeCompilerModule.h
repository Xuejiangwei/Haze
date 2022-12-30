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

	std::shared_ptr<HazeCompilerFunction> GetFunction(HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerFunction> AddFunction(HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param);


	std::shared_ptr<HazeCompilerValue> AddGlobalStringVariable(const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> AddDataVariable(HazeValue& Value);

	HazeValueType FindClass(const HAZE_STRING& ClassName);

public:
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

private:
	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param);

private:
	void GenASS_FileHeader();
	void GenASS_Instruction();
	void GenASS_GlobalData();

private:
	std::wofstream FS_Ass;
	std::wofstream FS_OpCode;

	int StackTop;

	HAZE_STRING CurrFunction;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerFunction>> MapGlobalFunction;

	unsigned int GlobalVariableSize;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerValue>> MapGlobalVariable;

	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerValue>> MapGlobalStringVariable;

	//³£Á¿
	std::unordered_map<long long, std::shared_ptr<HazeCompilerValue>> MapIntDataValue;
	std::unordered_map<bool, std::shared_ptr<HazeCompilerValue>> MapBoolDataValue;
	std::unordered_map<double, std::shared_ptr<HazeCompilerValue>> MapFloatDataValue;
};
