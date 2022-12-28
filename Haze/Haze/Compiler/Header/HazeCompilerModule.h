#pragma once

#include <unordered_map>
#include <fstream>

#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerFunction;

class HazeCompilerModule
{
public:
	HazeCompilerModule(const HAZE_STRING& ModuleName);
	~HazeCompilerModule();

	void GenBinaryFile();

	std::shared_ptr<HazeCompilerFunction> GetCurrFunction();

	std::shared_ptr<HazeCompilerFunction> GetFunction(HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerFunction> AddFunction(HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param);

	std::shared_ptr<HazeCompilerValue> AddGlobalVariable(const HazeDefineVariable& Var);

	std::shared_ptr<HazeCompilerValue> AddDataVariable(HazeValue& Value);

	HazeValueType FindClass(const HAZE_STRING& ClassName);

private:
	std::wofstream FS;

	HAZE_STRING CurrFunction;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerFunction>> MapGlobalFunction;

	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerValue>> MapGlobalVariable;
	std::unordered_map<long long, std::shared_ptr<HazeCompilerValue>> MapIntDataValue;
	std::unordered_map<bool, std::shared_ptr<HazeCompilerValue>> MapBoolDataValue;
	std::unordered_map<double, std::shared_ptr<HazeCompilerValue>> MapFloatDataValue;
};
