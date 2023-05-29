#pragma once

#include <fstream>
#include "Haze.h"
#include "ModuleUnit.h"

class HazeVM;

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

enum ExeFileType : uint8
{
	Out,
	In,
};

class HazeExecuteFile
{
public:
	HazeExecuteFile(ExeFileType Type);
	~HazeExecuteFile();

public:
	void WriteModule(const std::unordered_map<HAZE_STRING, std::shared_ptr<ModuleUnit>>& Module);

	void WriteExecuteFile(const ModuleUnit::GlobalDataTable& GlobalDataTable, const ModuleUnit::StringTable& StringTable,
		const ModuleUnit::ClassTable& ClassTable, const ModuleUnit::FunctionTable& FunctionTable);

	void ReadExecuteFile(HazeVM* VM);

	void ReadModule(HazeVM* VM);

	void CheckAll();
private:
	void WriteGlobalDataTable(const ModuleUnit::GlobalDataTable& Table);

	void WriteStringTable(const ModuleUnit::StringTable& Table);

	void WriteClassTable(const ModuleUnit::ClassTable& Table);

	void WriteFunctionTable(const ModuleUnit::FunctionTable& Table);

	void WriteFunctionInstruction(const ModuleUnit::FunctionTable& Table, uint32 Length);

	void WriteInstruction(const ModuleUnit::FunctionInstruction& Instruction);

private:
	void ReadGlobalDataTable(HazeVM* VM);

	void ReadStringTable(HazeVM* VM);

	void ReadClassTable(HazeVM* VM);

	void ReadFunctionTable(HazeVM* VM);

	void ReadFunctionInstruction(HazeVM* VM);
	
	void ReadInstruction(Instruction& Instruction);
private:
	std::unique_ptr<HAZE_BINARY_OFSTREAM> FileStream;
	std::unique_ptr<HAZE_BINARY_IFSTREAM> InFileStream;

	bool State[HazeFileFormat::End];
};

