#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Haze.h"
#include "HazeModule.h"

class HazeCompiler;

class HazeStack;

class HazeVM
{
public:
	HazeVM();
	~HazeVM();

	using ModulePair = std::pair<HAZE_STRING, HAZE_STRING>;

	void InitVM(std::vector<ModulePair> Vector_ModulePath);

	void StartMainFunction();

	void ParseString(const HAZE_STRING& String);

	void ParseFile(const HAZE_STRING& FilePath, const HAZE_STRING& ModuleName);

	HazeValue* GetVirtualRegister(const HAZE_CHAR* Name);

	std::unique_ptr<HazeCompiler>& GetCompiler() { return Compiler; }

	const std::unordered_map<HAZE_STRING, std::unique_ptr<HazeModule>>& GetModules() const { return UnorderedMap_Module; }

public:
	struct FunctionData
	{
		HazeValueType Type;
		std::vector<std::pair<HAZE_STRING, HazeValue>> Vector_Param;
		unsigned int InstructionNum;
		unsigned int InstructionStartAddress;
	};

	struct Instruction
	{
		InstructionOpCode InsCode;
		std::vector<InstructionData> Operator;
	};

private:
	void LoadOpCodeFile();

	void ReadInstruction(HAZE_BINARY_IFSTREAM& B_IFS, Instruction& Instruction);

private:
	void RunInstruction(const Instruction& Ins);

private:
	//std::unordered_map<HAZE_STRING, std::unique_ptr<Module>> MapModule;
	std::unordered_set<HAZE_STRING> MapString;

	std::unique_ptr<HazeCompiler> Compiler;

	std::unique_ptr<HazeStack> VMStack;

	HazeValue FunctionReturn;

	std::unordered_map<HAZE_STRING, std::unique_ptr<HazeModule>> UnorderedMap_Module;

	std::vector<std::pair<HAZE_STRING, HazeValue>> Vector_GlobalData;

	std::vector<std::pair<HAZE_STRING, HAZE_STRING>> Vector_StringTable;

	std::vector<std::pair<HAZE_STRING, FunctionData>> Vector_FunctionTable;
	std::vector<Instruction> Vector_Instruction;
};
