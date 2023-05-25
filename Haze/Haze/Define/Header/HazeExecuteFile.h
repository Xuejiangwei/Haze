#pragma once

#include "Haze.h"
#include "ModuleUnit.h"

//字节码文件头部数据格式定义(模仿linux程序结构 堆区、栈区、全局数据区、只读数据区等)
enum HazeFileFormat : uint8
{
	GlobalDataTable,
	StringTable,
	ClassTable,
	FunctionTable,
	InstructionTable,
	End,
};

using GlobalDataTable = ModuleUnit::GlobalDataTable;
using StringTable = ModuleUnit::StringTable;

class HazeExecuteFile
{
public:
	HazeExecuteFile();
	~HazeExecuteFile();

public:
	void WriteGlobalDataTable(const GlobalDataTable& Table);

	void WriteStringTable(const StringTable& Table);

	void WriteClassTable();

	void WriteFunctionTable();

private:
	HAZE_BINARY_OFSTREAM FileStream;

	bool State[HazeFileFormat::End];
};

