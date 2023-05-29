#pragma once

#include "Haze.h"
#include <vector>

class ModuleUnit
{
public:
	ModuleUnit(const HAZE_STRING& Name);
	~ModuleUnit();

public:
	friend class BackendParse;
	friend class HazeExecuteFile;

	struct GlobalData
	{
		HAZE_STRING Name;
		HazeDefineType Type;
		HazeValue Value;
	};

	struct GlobalDataTable
	{
		std::vector<GlobalData> Vector_Data;

		GlobalDataTable()
		{
			Vector_Data.clear();
		}

		uint64 GetIndex(const HAZE_STRING& Name)
		{
			for (uint64 i = 0; i < Vector_Data.size(); i++)
			{
				if (Vector_Data[i].Name == Name)
				{
					return i;
				}
			}

			return (uint64)-1;
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
		HAZE_STRING Name;
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
		HAZE_STRING Name;
		HazeValueType Type;
		std::vector<HazeDefineVariable> Vector_Param;
		std::vector<HazeLocalVariable> Vector_Variable;
		std::vector<FunctionBlock> Vector_Block;
		std::vector<FunctionInstruction> Vector_Instruction;
		InstructionFunctionType DescType;

		FunctionTableData()
		{
			Type = HazeValueType::Void;
			Vector_Param.clear();
			Vector_Block.clear();
			Vector_Instruction.clear();
			DescType = InstructionFunctionType::HazeFunction;
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
	bool IsStdLib;

	HAZE_STRING Name;

	GlobalDataTable Table_GlobalData;
	StringTable Table_String;
	ClassTable Table_Class;
	FunctionTable Table_Function;
};
