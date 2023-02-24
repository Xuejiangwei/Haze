#pragma once

#include "Haze.h"
#include <vector>

class ModuleUnit
{
public:
	ModuleUnit();
	~ModuleUnit();

public:
	friend class BackendParse;

	struct GlobalData
	{
		HAZE_STRING Name;
		HazeValue Value;
	};

	struct GlobalDataTable
	{
		unsigned int Num;
		std::vector<GlobalData> Vector_Data;

		GlobalDataTable()
		{
			Num = 0;
			Vector_Data.clear();
		}
	};

public:
	struct StringTableData
	{
		HAZE_STRING Name;
		HAZE_STRING String;
	};

	struct StringTable
	{
		unsigned int Num;
		std::vector<StringTableData> Vector_Data;

		StringTable()
		{
			Num = 0;
			Vector_Data.clear();
		}
	};

public:
	struct FunctionInstruction
	{
		InstructionOpCode InsCode;
		std::vector<std::pair<InstructionDataType, HAZE_STRING>> Operator;
	};

	struct FunctionTableData
	{
		HAZE_STRING Name;
		HazeValueType Type;
		std::vector<std::pair<HAZE_STRING, HazeValue>> Vector_Param;
		std::vector<FunctionInstruction> Vector_Instruction;

		FunctionTableData()
		{
			Vector_Param.clear();
			Vector_Instruction.clear();
		}
	};

	struct FunctionTable
	{
		unsigned int Num;
		std::vector<FunctionTableData> Vector_Data;

		FunctionTable()
		{
			Num = 0;
			Vector_Data.clear();
		}
	};

private:
	GlobalDataTable Table_GlobalData;
	StringTable Table_String;
	FunctionTable Table_Function;
};
