#pragma once

#include "HazeHeader.h"
#include <vector>

class ModuleUnit
{
public:
	ModuleUnit(const HAZE_STRING& name);

	~ModuleUnit();

public:
	friend class BackendParse;
	friend class HazeExecuteFile;

	struct FunctionInstruction
	{
		InstructionOpCode InsCode;
		std::vector<InstructionData> Operator;

		FunctionInstruction() : InsCode(InstructionOpCode::NONE), Operator()
		{
		}
	};

	struct GlobalData
	{
		HAZE_STRING Name;
		uint32 Size;
		HazeDefineType Type;
		HazeValue Value;

		uint32 StartAddress;
		uint32 EndAddress;
	};

	struct GlobalDataTable
	{
		std::vector<GlobalData> Data;
		uint32 ClassObjectAllSize;
		std::vector<FunctionInstruction> Instructions;

		GlobalDataTable()
		{
			Data.clear();
			ClassObjectAllSize = 0;
		}

		int GetIndex(const HAZE_STRING& name)
		{
			for (uint64 i = 0; i < Data.size(); i++)
			{
				if (Data[i].Name == name)
				{
					return (int)i;
				}
			}

			return -1;
		}
	};

public:
	struct StringTableData
	{
		//HAZE_STRING Name;
		HAZE_STRING String;
	};

	struct StringTable
	{
		std::vector<StringTableData> Strings;

		StringTable()
		{
			Strings.clear();
		}
	};

public:
	struct ClassMemberData
	{
		HazeDefineVariable Variable;
		uint32 Offset;
		uint32 Size;
	};

	struct ClassTableData
	{
		HAZE_STRING m_Name;
		uint32 Size;
		std::vector<ClassMemberData> Members;
	};

	struct ClassTable
	{
		std::vector<ClassTableData> Classes;
	};

public:

	struct FunctionBlock
	{
		HAZE_STRING BlockName;
		int InstructionNum;
		int StartAddress;
	};

	struct FunctionTableData
	{
		HAZE_STRING Name;
		HazeDefineType Type;
		std::vector<HazeDefineVariable> Params;
		std::vector<HazeVariableData> Variables;
		std::vector<FunctionBlock> Blocks;
		std::vector<FunctionInstruction> Instructions;
		InstructionFunctionType DescType;
		
		uint32 StartLine;
		uint32 EndLine;

		FunctionTableData()
		{
			Type = HazeDefineType();
			Params.clear();
			Blocks.clear();
			Instructions.clear();
			DescType = InstructionFunctionType::HazeFunction;
			StartLine = 0;
			EndLine = 0;
		}
	};

	struct FunctionTable
	{
		std::vector<FunctionTableData> m_Functions;

		FunctionTable()
		{
			m_Functions.clear();
		}
	};

private:
	HazeLibraryType m_LibraryType;

	HAZE_STRING m_Name;

	GlobalDataTable m_GlobalDataTable;
	StringTable m_StringTable;
	ClassTable m_ClassTable;
	FunctionTable m_FunctionTable;
};
