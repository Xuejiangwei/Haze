#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class HazeCompiler;

//class Module;
class HazeClass;

struct VirtualRegister
{
	uint64_t Data;
};

class HazeVM
{
public:
	HazeVM();
	~HazeVM();

	void ParseString(const std::wstring& String);

	void ParseFile(const std::wstring& FilePath);

	VirtualRegister* GetVirtualRegister(int64_t Index);

	std::unique_ptr<HazeCompiler>& GetCompiler() { return Compiler; }

private:
	//std::unordered_map<std::wstring, std::unique_ptr<Module>> MapModule;
	std::unordered_map<std::wstring, std::unique_ptr<HazeClass>> MapClass;
	std::unordered_set<std::wstring> MapString;

	VirtualRegister AX, BX, CX, DX;

	std::unique_ptr<HazeCompiler> Compiler;
};
