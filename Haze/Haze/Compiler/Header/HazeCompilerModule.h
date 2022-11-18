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

	HazeCompilerValue* AddGlobalVariable();

	HazeCompilerValue* AddLocalVariable();

private:
	std::wfstream FS;

	std::unique_ptr<HazeCompilerFunction> CurrFunction;
	std::unordered_map<HAZE_STRING, HazeCompilerValue> MapGlobalVariable;
};
