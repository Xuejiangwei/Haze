#include "HazeCompiler.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerModule.h"

HazeCompiler::HazeCompiler()
{
}

HazeCompiler::~HazeCompiler()
{
}

bool HazeCompiler::InitializeCompiler(const HAZE_STRING& ModuleName)
{
	//Éú³ÉÄ£¿é
	auto It = MapModules.find(ModuleName);
	if (It != MapModules.end())
	{
		return false;
	}

	CurrModule = ModuleName;
	MapModules[CurrModule] = std::make_unique<HazeCompilerModule>(ModuleName);
	return true;
}

HazeCompilerValue* HazeCompiler::GenGlobalVariable()
{
	std::unique_ptr<HazeCompilerModule>& Module = GetCurrModule();
	return Module->AddGlobalVariable();
}

HazeCompilerValue* HazeCompiler::GenLocalVariable()
{
	std::unique_ptr<HazeCompilerModule>& Module = GetCurrModule();
	return Module->AddGlobalVariable();
}

HazeCompilerValue* HazeCompiler::GetGlobalVariable(const HAZE_STRING& Name)
{
	std::unique_ptr<HazeCompilerModule>& Module = GetCurrModule();
	return nullptr;
}

HazeCompilerValue* HazeCompiler::GetLocalVariable(const HAZE_STRING& Name)
{
	return nullptr;
}