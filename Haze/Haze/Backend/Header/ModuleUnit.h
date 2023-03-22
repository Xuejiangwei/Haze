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
		std::vector<GlobalData> Vector_Data;

		GlobalDataTable()
		{
			Vector_Data.clear();
		}

		unsigned int GetIndex(const HAZE_STRING& Name)
		{
			for (size_t i = 0; i < Vector_Data.size(); i++)
			{
				if (Vector_Data[i].Name == Name)
				{
					return (unsigned int)i;
				}
			}

			return 0;
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
		std::vector<StringTableData> Vector_Data;

		StringTable()
		{
			Vector_Data.clear();
		}
	};

public:
	
	struct FunctionInstruction
	{
		InstructionOpCode InsCode;
		std::vector<InstructionData> Operator;
	};

	struct FunctionTableData
	{
		HAZE_STRING Name;
		HazeValueType Type;
		std::vector<std::pair<HAZE_STRING, HazeValue>> Vector_Param;
		std::vector<FunctionInstruction> Vector_Instruction;
		InstructionFunctionType DescType;

		FunctionTableData()
		{
			Type = HazeValueType::Void;
			Vector_Param.clear();
			Vector_Instruction.clear();
		}
	};

	struct FunctionTable
	{
		std::vector<FunctionTableData> Vector_Data;

		FunctionTable()
		{
			Vector_Data.clear();
		}
	};

private:
	bool IsStdLib;

	GlobalDataTable Table_GlobalData;
	StringTable Table_String;
	FunctionTable Table_Function;
};
