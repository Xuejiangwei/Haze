#pragma once

#include "Haze.h"
#include <vector>

class ModuleUnit
{
public:
	ModuleUnit(const HAZE_STRING& m_Name);
	~ModuleUnit();

public:
	friend class BackendParse;
	friend class HazeExecuteFile;

	struct GlobalData
	{
		HAZE_STRING m_Name;
		uint32 Size;
		HazeDefineType Type;
		HazeValue m_Value;
	};

	struct GlobalDataTable
	{
		std::vector<GlobalData> Vector_Data;
		uint32 ClassObjectAllSize;

		GlobalDataTable()
		{
			Vector_Data.clear();
			ClassObjectAllSize = 0;
		}

		int GetIndex(const HAZE_STRING& m_Name)
		{
			for (uint64 i = 0; i < Vector_Data.size(); i++)
			{
				if (Vector_Data[i].m_Name == m_Name)
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
		std::vector<StringTableData> Vector_String;

		StringTable()
		{
			Vector_String.clear();
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
		std::vector<ClassMemberData> Vector_Member;
	};

	struct ClassTable
	{
		std::vector<ClassTableData> Vector_Class;
	};

public:

	struct FunctionInstruction
	{
		InstructionOpCode InsCode;
		std::vector<InstructionData> Operator;

		FunctionInstruction() : InsCode(InstructionOpCode::NONE), Operator()
		{
		}
	};

	struct FunctionBlock
	{
		HAZE_STRING BlockName;
		int InstructionNum;
		int StartAddress;
	};

	struct FunctionTableData
	{
		HAZE_STRING m_Name;
		HazeValueType Type;
		std::vector<HazeDefineVariable> Vector_Param;
		std::vector<HazeVariableData> m_Vector_Variables;
		std::vector<FunctionBlock> Vector_Block;
		std::vector<FunctionInstruction> Vector_Instruction;
		InstructionFunctionType DescType;
		
		uint32 StartLine;
		uint32 EndLine;

		FunctionTableData()
		{
			Type = HazeValueType::Void;
			Vector_Param.clear();
			Vector_Block.clear();
			Vector_Instruction.clear();
			DescType = InstructionFunctionType::HazeFunction;
			StartLine = 0;
			EndLine = 0;
		}
	};

	struct FunctionTable
	{
		std::vector<FunctionTableData> Vector_Function;

		FunctionTable()
		{
			Vector_Function.clear();
		}
	};

private:
	HazeLibraryType LibraryType;

	HAZE_STRING m_Name;

	GlobalDataTable Table_GlobalData;
	StringTable Table_String;
	ClassTable Table_Class;
	FunctionTable Table_Function;
};
