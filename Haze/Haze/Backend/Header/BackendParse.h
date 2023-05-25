#pragma once

#include <fstream>

#include "Haze.h"
#include "ModuleUnit.h"

class HazeVM;
class HazeExecuteFile;

class BackendParse
{
public:
	BackendParse(HazeVM* VM);
	~BackendParse();

	void Parse();

public:
	unsigned int const GetClassSize(const HAZE_STRING& ClassName);

private:
	void GetNextLexeme();

	void GetNextLexmeAssign_HazeString(HAZE_STRING& Dst)
	{
		GetNextLexeme();
		Dst = CurrLexeme;
	};

	template<typename T>
	void GetNextLexmeAssign_StandardType(T& Dst)
	{
		GetNextLexeme();
		Dst = StringToStandardType<T>(CurrLexeme);
	};

	template<typename Type, typename T>
	void GetNextLexmeAssign_CustomType(T& Dst)
	{
		GetNextLexeme();
		Dst = (T)StringToStandardType<Type>(CurrLexeme);
	};

	void Parse_I_Code();

	void Parse_I_Code_GlobalTable();

	void Parse_I_Code_StringTable();
	
	void Parse_I_Code_ClassTable();
	
	void Parse_I_Code_FunctionTable();

	void ParseInstructionData(InstructionData& Data, bool ParsePrimaryType = true);
	
	void ParseInstruction(ModuleUnit::FunctionInstruction& Instruction);

	void GenOpCodeFile();

	void ReplaceStringIndex(ModuleUnit::StringTable& NewStringTable, ModuleUnit::FunctionTable& NewFunctionTable, size_t& FunctionCount);
	
	void FindAddress(ModuleUnit::GlobalDataTable& NewGlobalDataTable, ModuleUnit::FunctionTable& NewFunctionTable);

	void WriteInstruction(HAZE_BINARY_OFSTREAM& B_OFS, ModuleUnit::FunctionInstruction& Instruction);

	const ModuleUnit::ClassTableData* const GetClass(const HAZE_STRING& ClassName);

	uint32 GetMemberOffset(const ModuleUnit::ClassTableData& Class, const HAZE_STRING& MemberName);
	
private:
	HazeVM* VM;

	const HAZE_CHAR* CurrCode;
	HAZE_STRING CurrLexeme;

	std::shared_ptr<ModuleUnit> CurrParseModule;
	std::unordered_map<HAZE_STRING, std::shared_ptr<ModuleUnit>> HashMap_Modules;

	std::unique_ptr<HazeExecuteFile> ExeFile;
};
