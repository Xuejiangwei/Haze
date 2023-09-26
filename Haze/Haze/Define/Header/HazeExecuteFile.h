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
	HazeExecuteFile(ExeFileType type);

	~HazeExecuteFile();

public:
	void WriteModule(const std::unordered_map<HAZE_STRING, std::shared_ptr<ModuleUnit>>& moduleUnit);

	void WriteExecuteFile(const ModuleUnit::GlobalDataTable& globalDataTable, const ModuleUnit::StringTable& stringTable,
		const ModuleUnit::ClassTable& classTable, const ModuleUnit::FunctionTable& functionTable);

	void ReadExecuteFile(HazeVM* vm);

	void ReadModule(HazeVM* vm);

	void CheckAll();

private:
	void WriteGlobalDataTable(const ModuleUnit::GlobalDataTable& table);

	void WriteStringTable(const ModuleUnit::StringTable& table);

	void WriteClassTable(const ModuleUnit::ClassTable& table);

	void WriteFunctionTable(const ModuleUnit::FunctionTable& table);

	void WriteFunctionInstruction(const ModuleUnit::FunctionTable& table, uint32 length);

	void WriteInstruction(const ModuleUnit::FunctionInstruction& instruction);

private:
	void ReadGlobalDataTable(HazeVM* vm);

	void ReadStringTable(HazeVM* vm);

	void ReadClassTable(HazeVM* vm);

	void ReadFunctionTable(HazeVM* vm);

	void ReadFunctionInstruction(HazeVM* vm);

	void ReadInstruction(Instruction& instruction);

private:
	std::unique_ptr<HAZE_BINARY_OFSTREAM> m_FileStream;
	std::unique_ptr<HAZE_BINARY_IFSTREAM> m_InFileStream;

	bool m_States[HazeFileFormat::End];
};
