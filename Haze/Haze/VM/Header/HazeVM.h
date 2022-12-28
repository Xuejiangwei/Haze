#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Haze.h"

class HazeCompiler;

class HazeStack;

class HazeVM
{
public:
	HazeVM();
	~HazeVM();

	void ParseString(const HAZE_STRING& String);

	void ParseFile(const HAZE_STRING& FilePath, const HAZE_STRING& ModuleName);

	HazeValue* GetVirtualRegister(uint64_t Index);

	std::unique_ptr<HazeCompiler>& GetCompiler() { return Compiler; }

private:
	//std::unordered_map<HAZE_STRING, std::unique_ptr<Module>> MapModule;
	std::unordered_set<HAZE_STRING> MapString;

	std::unique_ptr<HazeCompiler> Compiler;

	std::unique_ptr<HazeStack> VMStack;

	HazeValue FunctionReturn;

	int PC; //��ǰָ��
};
