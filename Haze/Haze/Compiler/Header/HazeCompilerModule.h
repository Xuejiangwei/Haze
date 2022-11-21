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

	HazeCompilerValue* AddGlobalVariable(const HAZE_STRING& Name, HazeValueType Type);

	HazeCompilerValue* AddLocalVariable();

	HazeCompilerValue* AddDataVariable(HazeValue& Value);

private:
	std::wofstream FS;

	std::unique_ptr<HazeCompilerFunction> CurrFunction;
	std::unordered_map<HAZE_STRING, HazeCompilerValue> MapGlobalVariable;
	std::unordered_map<long long, HazeCompilerValue> MapIntDataValue;
	std::unordered_map<bool, HazeCompilerValue> MapBoolDataValue;
	std::unordered_map<double, HazeCompilerValue> MapFloatDataValue;
};
