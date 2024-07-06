#pragma once

#include "ModuleUnit.h"

class HazeVM;
class HazeExecuteFile;

class BackendParse
{
public:
	BackendParse(HazeVM* vm);

	~BackendParse();

	void Parse();

public:
	unsigned int const GetClassSize(const HString& className);

private:
	void GetNextLexeme();

	void GetNextLexmeAssign_HazeString(HString& dst)
	{
		GetNextLexeme();
		dst = m_CurrLexeme;
	};

	template<typename T>
	void GetNextLexmeAssign_StandardType(T& dst)
	{
		GetNextLexeme();
		dst = StringToStandardType<T>(m_CurrLexeme);
	};

	template<typename m_Type, typename T>
	void GetNextLexmeAssign_CustomType(T& dst)
	{
		GetNextLexeme();
		dst = (T)StringToStandardType<m_Type>(m_CurrLexeme);
	};

	void Parse_I_Code();

	void Parse_I_Code_GlobalTable();

	void Parse_I_Code_StringTable();

	void Parse_I_Code_ClassTable();

	void Parse_I_Code_FunctionTable();

	void ParseInstructionData(InstructionData& data);

	void ParseInstruction(ModuleUnit::FunctionInstruction& instruction);

	void GenOpCodeFile();

	void ReplaceStringIndex(ModuleUnit::StringTable& newStringTable, 
		ModuleUnit::FunctionTable& newFunctionTable, size_t& functionCount);

	inline void ResetLocalOperatorAddress(InstructionData& operatorData, ModuleUnit::FunctionTableData& function,
		HashMap<HString, int>& localVariable, ModuleUnit::GlobalDataTable& newGlobalDataTable);

	inline void ResetGlobalOperatorAddress(InstructionData& operatorData, ModuleUnit::GlobalDataTable& newGlobalDataTable);

	void FindAddress(ModuleUnit::GlobalDataTable& newGlobalDataTable, ModuleUnit::FunctionTable& newFunctionTable);

	const ModuleUnit::ClassTableData* const GetClass(const HString& className);

	uint32 GetMemberOffset(const ModuleUnit::ClassTableData& classData, const HString& memberName);

private:
	HazeVM* m_VM;

	const HChar* m_CurrCode;
	HString m_CurrLexeme;

	Share<ModuleUnit> m_CurrParseModule;
	HashMap<HString, Share<ModuleUnit>> m_Modules;
};
