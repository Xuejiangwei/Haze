#pragma once

#include "Haze.h"
#include "ModuleUnit.h"

//�ֽ����ļ�ͷ�����ݸ�ʽ����(ģ��linux����ṹ ������ջ����ȫ����������ֻ����������)
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

