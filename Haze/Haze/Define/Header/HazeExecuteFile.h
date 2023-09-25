#pragma once

#include <fstream>
#include "Haze.h"
#include "ModuleUnit.h"

class HazeVM;

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

enum ExeFileType : uint8
{
	Out,
	In,
};

class HazeExecuteFile
{
public:
	HazeExecuteFile(ExeFileType m_Type);
	~HazeExecuteFile();

public:
	void WriteModule(const std::unordered_map<HAZE_STRING, std::shared_ptr<ModuleUnit>>& m_Module);

	void WriteExecuteFile(const ModuleUnit::GlobalDataTable& GlobalDataTable, const ModuleUnit::StringTable& StringTable,
		const ModuleUnit::ClassTable& ClassTable, const ModuleUnit::FunctionTable& FunctionTable);

	void ReadExecuteFile(HazeVM* m_VM);

	void ReadModule(HazeVM* m_VM);

	void CheckAll();
private:
	void WriteGlobalDataTable(const ModuleUnit::GlobalDataTable& Table);

	void WriteStringTable(const ModuleUnit::StringTable& Table);

	void WriteClassTable(const ModuleUnit::ClassTable& Table);

	void WriteFunctionTable(const ModuleUnit::FunctionTable& Table);

	void WriteFunctionInstruction(const ModuleUnit::FunctionTable& Table, uint32 Length);

	void WriteInstruction(const ModuleUnit::FunctionInstruction& Instruction);

private:
	void ReadGlobalDataTable(HazeVM* m_VM);

	void ReadStringTable(HazeVM* m_VM);

	void ReadClassTable(HazeVM* m_VM);

	void ReadFunctionTable(HazeVM* m_VM);

	void ReadFunctionInstruction(HazeVM* m_VM);

	void ReadInstruction(Instruction& Instruction);
private:
	std::unique_ptr<HAZE_BINARY_OFSTREAM> FileStream;
	std::unique_ptr<HAZE_BINARY_IFSTREAM> InFileStream;

	bool State[HazeFileFormat::End];
};
