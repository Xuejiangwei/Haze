#pragma once

#include <fstream>

#include "Haze.h"
#include "ModuleUnit.h"

class HazeVM;

class BackendParse
{
public:
	BackendParse(HazeVM* VM);
	~BackendParse();

	void Parse();

private:
	void GetNextLexeme();

	void Parse_I_Code();

	void ParseInstruction(ModuleUnit::FunctionInstruction& Instruction);

	void GenOpCodeFile();

	void WriteInstruction(HAZE_BINARY_OFSTREAM& B_OFS, ModuleUnit::FunctionInstruction& Instruction);

private:
	HazeVM* VM;

	HAZE_BINARY_OFSTREAM FS_OpCode;

	const HAZE_CHAR* CurrCode = nullptr;
	HAZE_STRING CurrLexeme;

	std::shared_ptr<ModuleUnit> CurrParseModule;
	std::unordered_map<HAZE_STRING, std::shared_ptr<ModuleUnit>> HashMap_Modules;
};
