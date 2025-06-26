#pragma once

class ModuleUnit
{
public:
	ModuleUnit(const HString& name);

	~ModuleUnit();

public:
	friend class BackendParse;
	friend class HazeExecuteFile;

	struct FunctionInstruction
	{
		InstructionOpCode InsCode;
		V_Array<InstructionData> Operator;

		FunctionInstruction() : InsCode(InstructionOpCode::NONE), Operator()
		{
		}
	};

	struct GlobalData
	{
		HString Name;
		HazeDefineType Type;

		/*uint32 StartAddress;
		uint32 EndAddress;*/
	};

	struct GlobalDataTable
	{
		V_Array<GlobalData> Data;
		V_Array<x_uint64> InitFunctionIndex;

		GlobalDataTable()
		{
			Data.clear();
			InitFunctionIndex.clear();
		}

		int GetIndex(const HString& name)
		{
			for (x_uint64 i = 0; i < Data.size(); i++)
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
		//HString Name;
		HString String;
	};

	struct StringTable
	{
		V_Array<StringTableData> Strings;

		StringTable()
		{
			Strings.clear();
		}
	};

public:
	struct ClassMemberData
	{
		HazeDefineVariable Variable;
		x_uint32 Offset;
		x_uint32 Size;
	};

	struct ClassTableData
	{
		HString Name;
		x_uint32 Size;
		V_Array<ClassMemberData> Members;
		V_Array<HString> ParentClasses;
	};

	struct ClassTable
	{
		V_Array<ClassTableData> Classes;
		HashMap<HString, x_uint32> IndexMap;
	};

public:
	struct FunctionBlock
	{
		HString BlockName;
		int InstructionNum;
		int StartAddress;
	};

	struct FunctionTableData
	{
		HString ClassName;
		HString Name;
		HazeDefineType Type;
		V_Array<HazeDefineVariable> Params;
		V_Array<HazeVariableData> Variables;
		V_Array<HazeTempRegisterData> TempRegisters;
		V_Array<FunctionBlock> Blocks;
		V_Array<FunctionInstruction> Instructions;
		V_Array<Pair<int, int>> RefVariable;			//±Õ°üÊ¹ÓÃ
		InstructionFunctionType DescType;
		
		x_uint32 StartLine;
		x_uint32 EndLine;

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
		V_Array<FunctionTableData> m_Functions;

		FunctionTable()
		{
			m_Functions.clear();
		}
	};

private:
	HazeLibraryType m_LibraryType;
	HString m_Path;
	HString m_Name;

	GlobalDataTable m_GlobalDataTable;
	StringTable m_StringTable;
	ClassTable m_ClassTable;
	FunctionTable m_FunctionTable;
};
