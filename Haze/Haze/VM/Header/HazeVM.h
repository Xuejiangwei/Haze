#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Haze.h"

class HazeCompiler;

//class Module;

struct VirtualRegister
{
	uint64_t Data;
};

class HazeVM
{
public:
	HazeVM();
	~HazeVM();

	void ParseString(const HAZE_STRING& String);

	void ParseFile(const HAZE_STRING& FilePath, const HAZE_STRING& ModuleName);

	VirtualRegister* GetVirtualRegister(int64_t Index);

	std::unique_ptr<HazeCompiler>& GetCompiler() { return Compiler; }

private:
	//std::unordered_map<std::wstring, std::unique_ptr<Module>> MapModule;
	std::unordered_set<std::wstring> MapString;

	VirtualRegister AX, BX, CX, DX;

	std::unique_ptr<HazeCompiler> Compiler;
};
