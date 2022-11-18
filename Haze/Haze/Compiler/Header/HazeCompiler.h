#pragma once

#include <memory>
#include <unordered_map>

#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerModule;

class HazeCompiler
{
public:
	HazeCompiler();
	~HazeCompiler();

	bool InitializeCompiler(const HAZE_STRING& ModuleName);

	std::unique_ptr<HazeCompilerModule>& GetCurrModule() { return MapModules[CurrModule]; }

	HazeCompilerValue* GenGlobalVariable();

	HazeCompilerValue* GenLocalVariable();

	HazeCompilerValue* GetGlobalVariable(const HAZE_STRING& Name);

	HazeCompilerValue* GetLocalVariable(const HAZE_STRING& Name);

private:
	HAZE_STRING CurrModule;

	std::unordered_map<HAZE_STRING, std::unique_ptr<HazeCompilerModule>> MapModules;
	std::unordered_map<HAZE_STRING, HazeCompilerValue> GlobalVarialbs;
	std::unordered_map<HAZE_STRING, HazeCompilerValue> GlobalFunctions;
};
