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
	x_uint32 const GetClassSize(const STDString& className);

private:
	void GetNextLexeme();

	void GetNextLexmeAssign_HazeString(STDString& dst)
	{
		GetNextLexeme();
		dst = m_CurrLexeme;
	};

	void GetNextLexmeAssign_HazeString(HString& dst)
	{
		GetNextLexeme();
		dst = m_CurrLexeme;
	};

	void GetNextLexmeAssign_HazeStringCustomClassName(const STDString*& dst);

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

	//void Parse_I_Symbol();

	void Parse_I_TypeInfo();

	void Parse_I_Code();

	void Parse_I_Code_ImportModule();

	void Parse_I_Code_ImportTable();

	void Parse_I_Code_GlobalTable();

	void Parse_I_Code_StringTable();

	void Parse_I_Code_EnumTable();

	void Parse_I_Code_ClassTable();

	void Parse_I_Code_FunctionTable();

	void ParseInstructionData(InstructionData& data);

	void ParseInstruction(ModuleUnit::FunctionInstruction& instruction);

	void GenOpCodeFile();

	void ReplaceStringIndex(ModuleUnit::StringTable& newStringTable, 
		ModuleUnit::FunctionTable& newFunctionTable, size_t& functionCount);

	inline void ResetLocalOperatorAddress(InstructionData& operatorData, ModuleUnit::FunctionTableData& function,
		HashMap<STDString, int>& localVariable, HashMap<STDString, int> tempRegister);

	inline void ResetGlobalOperatorAddress(InstructionData& operatorData, ModuleUnit::GlobalDataTable& newGlobalDataTable);

	void FindAddress(ModuleUnit::GlobalDataTable& newGlobalDataTable, ModuleUnit::FunctionTable& newFunctionTable);

	const ModuleUnit::ClassTableData* const GetClass(const STDString& className);

	x_uint32 GetClassIndex(const STDString& className);

	x_uint32 GetMemberOffset(const ModuleUnit::ClassTableData& classData, const STDString& memberName);

private:
	HazeVM* m_VM;

	const x_HChar* m_CurrCode;
	STDString m_CurrLexeme;

	Share<ModuleUnit> m_CurrParseModule;
	HashMap<STDString, Share<ModuleUnit>> m_Modules;

	HashSet<STDString> m_InterSymbol;

	V_Array<Pair<x_uint32, V_Array<x_uint32>>> m_TypeInfoFunction;
	V_Array<Pair<STDString, Pair<x_uint32, HazeComplexTypeInfo>>> m_TypeInfo;
};
