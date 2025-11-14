#pragma once

class ModuleUnit
{
	friend class Optimizer;
public:
	ModuleUnit(const STDString& name);

	~ModuleUnit();

public:
	friend class BackendParse;
	friend class HazeExecuteFile;

	struct FunctionInstruction
	{
		InstructionOpCode InsCode;

#if HAZE_DEBUGGER
		x_uint32 Line;
#endif // HAZE_DEBUGGER

		V_Array<InstructionData> Operator;

		FunctionInstruction() : InsCode(InstructionOpCode::NONE), Operator()
		{
		}
	};

	struct GlobalData
	{
		STDString Name;
		x_uint32 Id;
		HazeVariableType Type;

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

		int GetIndex(const STDString& name)
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
		STDString String;
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
		STDString Name;
		x_uint32 Size;
		x_uint32 TypeId;
		V_Array<ClassMemberData> Members;
		V_Array<STDString> ParentClasses;
	};

	struct ClassTable
	{
		V_Array<ClassTableData> Classes;
		HashMap<STDString, x_uint32> IndexMap;
	};

public:
	struct FunctionBlock
	{
		STDString BlockName;
		int InstructionNum;
		int StartAddress;

		V_Array<x_uint32> Predecessors;
		V_Array<x_uint32> Successors;
	};

	struct FunctionTableData
	{
		STDString ClassName;
		STDString Name;
		HazeVariableType Type;
		V_Array<HazeDefineVariable> Params;
		V_Array<HazeVariableData> Variables;
		V_Array<HazeTempRegisterData> TempRegisters;
		V_Array<FunctionBlock> Blocks;
		V_Array<FunctionInstruction> Instructions;
		V_Array<Pair<int, int>> RefVariable;			//闭包使用
		InstructionFunctionType DescType;
		
		x_uint32 StartLine;
		x_uint32 EndLine;

		FunctionTableData()
		{
			Type = HazeVariableType();
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
	STDString m_Path;
	STDString m_Name;

	GlobalDataTable m_GlobalDataTable;
	StringTable m_StringTable;
	ClassTable m_ClassTable;
	FunctionTable m_FunctionTable;
};
