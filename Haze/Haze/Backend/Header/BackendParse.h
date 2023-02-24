#pragma once

#include "Haze.h"

class HazeVM;
class ModuleUnit;

class BackendParse
{
public:
	BackendParse(HazeVM* VM);
	~BackendParse();

	void Parse();

private:
	void ToParse();

	void ParseHeader();

	void ParseInstruction();

	InstructionOpCode GetNextInstruction();

private:
	HazeVM* VM;

	const HAZE_CHAR* CurrCode = nullptr;
	InstructionOpCode CurrInstruction;
	HAZE_STRING CurrLexeme;

	std::shared_ptr<ModuleUnit> CurrParseModule;
	std::unordered_map<HAZE_STRING, std::shared_ptr<ModuleUnit>> HashMap_Modules;
};
