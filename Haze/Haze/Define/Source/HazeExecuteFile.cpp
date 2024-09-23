#include "HazePch.h"

#include "HazeExecuteFile.h"
#include "HazeStandardLibraryBase.h"
#include "HazeVM.h"
#include "HazeFilePathHelper.h"
#include "HazeLog.h"

#define HAZE_INS_LOG			0

thread_local static HAZE_BINARY_STRING s_BinaryString;

static HashMap<HazeFileFormat, const HChar*> s_HashMap_FileFormatString =
{
	{HazeFileFormat::GlobalDataTable, H_TEXT("全局数据表")},
	{HazeFileFormat::StringTable, H_TEXT("字符串表")},
	{HazeFileFormat::ClassTable, H_TEXT("类表")},
	{HazeFileFormat::FunctionTable, H_TEXT("函数表")},
	{HazeFileFormat::InstructionTable, H_TEXT("指令表")},
};

static const HChar* GetFileFormatString(HazeFileFormat format)
{
	auto Iter = s_HashMap_FileFormatString.find(HazeFileFormat::GlobalDataTable);
	if (Iter != s_HashMap_FileFormatString.end())
	{
		return Iter->second;
	}

	HAZE_LOG_ERR_W("不能够找到二进制文件的格式<%d>!\n", (int)format);
	return H_TEXT("None");
}

struct FileFormatCheck
{
	FileFormatCheck(ExeFileType type, HazeFileFormat fileFormat, bool* state) : Format(fileFormat), State(state)
	{
		for (size_t i = 0; i < Format; i++)
		{
			if (!State[i] && i < Format)
			{
				if (type == ExeFileType::Out)
				{
					HAZE_LOG_ERR_W("生成执行文件错误,没有生成<%s>数据!\n", GetFileFormatString((HazeFileFormat)i));
				}
				else if (type == ExeFileType::In)
				{
					HAZE_LOG_ERR_W("解析执行文件错误,没有解析<%s>数据!\n", GetFileFormatString((HazeFileFormat)i));
				}
			}
		}

		if (State[Format])
		{
			if (type == ExeFileType::Out)
			{
				HAZE_LOG_ERR_W("生成执行文件错误,重复生成<%s>!\n", GetFileFormatString(Format));
			}
			else if (type == ExeFileType::In)
			{
				HAZE_LOG_ERR_W("解析执行文件错误,重复解析<%s>!\n", GetFileFormatString(Format));
			}
		}
	}

	~FileFormatCheck()
	{
		State[Format] = true;
	}

	bool HasCheck() { return State[Format]; }

private:
	bool* State;
	HazeFileFormat Format;
};

inline void WriteType(Unique<HAZE_BINARY_OFSTREAM>& fileStream, const HazeDefineType& type)
{
	fileStream->write(HAZE_WRITE_AND_SIZE(type.PrimaryType));

	if (type.NeedSecondaryType())
	{
		fileStream->write(HAZE_WRITE_AND_SIZE(type.SecondaryType));
	}

	if (type.NeedCustomName())
	{
		s_BinaryString.clear();
		s_BinaryString = WString2String(*type.CustomName);
		auto n = (uint32)s_BinaryString.size();
		fileStream->write(HAZE_WRITE_AND_SIZE(n));
		fileStream->write(s_BinaryString.c_str(), n);
	}
}

inline void HazeExecuteFile::ReadType(HazeVM* vm, Unique<HAZE_BINARY_IFSTREAM>& fileStream, HazeDefineType& type)
{
	fileStream->read(HAZE_READ(type.PrimaryType));

	if (type.NeedSecondaryType())
	{
		fileStream->read(HAZE_READ(type.SecondaryType));
	}

	if (type.NeedCustomName())
	{
		uint32 n = 0;
		fileStream->read(HAZE_READ(n));
		s_BinaryString.resize(n);
		fileStream->read(s_BinaryString.data(), n);
		type.CustomName = vm->GetSymbolClassName(String2WString(s_BinaryString));
	}
}

HazeExecuteFile::HazeExecuteFile(ExeFileType type)
{
	if (type == ExeFileType::Out)
	{
		m_FileStream = MakeUnique<HAZE_BINARY_OFSTREAM>();
		//不用二进制的话，写入10，会当成换行特殊处理，写入两个字符 0x0d 0x0a，即回车换行符
		m_FileStream->open(GetMainBinaryFilePath(), std::ios::out | std::ios::binary);
	}
	else if (type == ExeFileType::In)
	{
		m_InFileStream = MakeUnique<HAZE_BINARY_IFSTREAM>(GetMainBinaryFilePath(), std::ios::in | std::ios::binary);
		m_InFileStream->imbue(std::locale("chs"));
	}
	else
	{
		HAZE_LOG_ERR_W("处理Haze二进制文件失败!\n");
	}

	memset(&m_States, 0, sizeof(m_States));
}

HazeExecuteFile::~HazeExecuteFile()
{
	if (m_FileStream && m_FileStream->is_open())
	{
		m_FileStream->close();
	}

	if (m_InFileStream && m_InFileStream->is_open())
	{
		m_InFileStream->close();
	}

	CheckAll();
}

void HazeExecuteFile::CheckAll()
{
	for (size_t i = 0; i < HazeFileFormat::End; i++)
	{
		if (!m_States[i])
		{
			if (m_FileStream)
			{
				auto it = s_HashMap_FileFormatString.find((HazeFileFormat)i);
				if (it != s_HashMap_FileFormatString.end())
				{
					HAZE_LOG_ERR_W("生成<%s>错误!\n", it->second);
				}
				else
				{
					HAZE_LOG_ERR_W("生成二进制文件错误\n");
				}
			}
			else
			{
				auto it = s_HashMap_FileFormatString.find((HazeFileFormat)i);
				if (it != s_HashMap_FileFormatString.end())
				{
					HAZE_LOG_ERR_W("解析<%s>错误!\n", it->second);
				}
				else
				{
					HAZE_LOG_ERR_W("解析二进制文件错误\n");
				}
			}
		}
	}
}

void HazeExecuteFile::WriteModule(const HashMap<HString, Share<ModuleUnit>>& moduleUnit)
{
	uint32 globalDataIndex = 0;
	uint32 stringIndex = 0;
	uint32 ClassIndex = 0;
	uint32 FunctionIndex = 0;

	uint32 number = (uint32)moduleUnit.size();
	m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

	for (auto& iter : moduleUnit)
	{
		s_BinaryString = WString2String(iter.first);
		number = (uint32)s_BinaryString.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
		m_FileStream->write(s_BinaryString.c_str(), number);

		m_FileStream->write(HAZE_WRITE_AND_SIZE(globalDataIndex));
		globalDataIndex += (uint32)iter.second->m_GlobalDataTable.Data.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(globalDataIndex));

		m_FileStream->write(HAZE_WRITE_AND_SIZE(stringIndex));
		stringIndex += (uint32)iter.second->m_StringTable.Strings.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(stringIndex));

		m_FileStream->write(HAZE_WRITE_AND_SIZE(ClassIndex));
		ClassIndex += (uint32)iter.second->m_ClassTable.Classes.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(ClassIndex));

		m_FileStream->write(HAZE_WRITE_AND_SIZE(FunctionIndex));
		FunctionIndex += (uint32)iter.second->m_FunctionTable.m_Functions.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(FunctionIndex));
	}
}

void HazeExecuteFile::WriteExecuteFile(const ModuleUnit::GlobalDataTable& globalDataTable, const ModuleUnit::StringTable& stringTable,
	const ModuleUnit::ClassTable& classTable, const ModuleUnit::FunctionTable& functionTable)
{
	WriteGlobalDataTable(globalDataTable);
	WriteStringTable(stringTable);
	WriteClassTable(classTable);
	auto funcInstructionLength = WriteFunctionTable(functionTable);
	WriteAllInstruction(functionTable, funcInstructionLength);
}

void HazeExecuteFile::WriteGlobalDataTable(const ModuleUnit::GlobalDataTable& table)
{
	FileFormatCheck check(ExeFileType::Out, HazeFileFormat::GlobalDataTable, m_States);

	
	uint32 number = (uint32)table.InitFunctionIndex.size();
	m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
	for (auto iter : table.InitFunctionIndex)
	{
		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter));
	}

	number = (uint32)table.Data.size();
	m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
	for (auto& iter : table.Data)
	{
		s_BinaryString = WString2String(iter.Name);
		number = (uint32)s_BinaryString.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
		m_FileStream->write(s_BinaryString.c_str(), number);
		WriteType(m_FileStream, iter.Type);
	}
}

void HazeExecuteFile::WriteStringTable(const ModuleUnit::StringTable& table)
{
	FileFormatCheck check(ExeFileType::Out, HazeFileFormat::StringTable, m_States);

	uint32 number = (uint32)table.Strings.size();
	m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

	for (auto& iter : table.Strings)
	{
		s_BinaryString = WString2String(iter.String);
		number = (uint32)s_BinaryString.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
		m_FileStream->write(s_BinaryString.data(), number);
	}
}

void HazeExecuteFile::WriteClassTable(const ModuleUnit::ClassTable& table)
{
	FileFormatCheck check(ExeFileType::Out, HazeFileFormat::ClassTable, m_States);

	uint32 number = (uint32)table.Classes.size();
	m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

	for (auto& iter : table.Classes)
	{
		s_BinaryString = WString2String(iter.m_Name);
		number = (uint32)s_BinaryString.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
		m_FileStream->write(s_BinaryString.data(), number);

		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Size));

		number = (uint32)iter.Members.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

		for (size_t i = 0; i < iter.Members.size(); i++)
		{
			s_BinaryString = WString2String(iter.Members[i].Variable.Name);
			number = (uint32)s_BinaryString.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			m_FileStream->write(s_BinaryString.data(), number);

			WriteType(m_FileStream, iter.Members[i].Variable.Type);
			m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Members[i].Offset));
			m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Members[i].Size));
		}
	}
}

uint64 HazeExecuteFile::WriteFunctionTable(const ModuleUnit::FunctionTable& table)
{
	uint64 instructionStartAddr = 0;
	{
		FileFormatCheck check(ExeFileType::Out, HazeFileFormat::FunctionTable, m_States);

		uint32 number = (uint32)table.m_Functions.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

		for (auto& function : table.m_Functions)
		{
			s_BinaryString = WString2String(function.ClassName);
			number = (uint32)s_BinaryString.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			m_FileStream->write(s_BinaryString.data(), number);

			s_BinaryString = WString2String(function.Name);
			number = (uint32)s_BinaryString.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			m_FileStream->write(s_BinaryString.data(), number);

			WriteType(m_FileStream, function.Type);
			m_FileStream->write(HAZE_WRITE_AND_SIZE(function.DescType));

			number = (uint32)function.Params.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			for (auto& iter : function.Params)
			{
				s_BinaryString = WString2String(iter.Name);
				number = (uint32)s_BinaryString.size();
				m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
				m_FileStream->write(s_BinaryString.c_str(), number);
				WriteType(m_FileStream, iter.Type);
			}

			number = (uint32)function.Variables.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			for (auto& iter : function.Variables)
			{
				s_BinaryString = WString2String(iter.Variable.Name);
				number = (uint32)s_BinaryString.size();
				m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
				m_FileStream->write(s_BinaryString.c_str(), number);
				WriteType(m_FileStream, iter.Variable.Type);
				
				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Offset));
				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Size));
				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Line));
			}

			number = (uint32)function.TempRegisters.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			for (auto& iter : function.TempRegisters)
			{
				s_BinaryString = WString2String(iter.Name);
				number = (uint32)s_BinaryString.size();
				m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
				m_FileStream->write(s_BinaryString.c_str(), number);
				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Offset));
				WriteType(m_FileStream, iter.Type);
			}

			//写入指令数与起始地址
			number = (uint32)function.Instructions.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

			m_FileStream->write(HAZE_WRITE_AND_SIZE(instructionStartAddr));
			instructionStartAddr += number;

			m_FileStream->write(HAZE_WRITE_AND_SIZE(function.StartLine));
			m_FileStream->write(HAZE_WRITE_AND_SIZE(function.EndLine));
		}
	}

	return instructionStartAddr;
}

void HazeExecuteFile::WriteAllInstruction(const ModuleUnit::FunctionTable& table, uint64 funcInslength)
{
	FileFormatCheck check(ExeFileType::Out, HazeFileFormat::InstructionTable, m_States);

	m_FileStream->write(HAZE_WRITE_AND_SIZE(funcInslength));
	for (auto& function : table.m_Functions)
	{
		for (auto& ins : function.Instructions)
		{
			WriteInstruction(ins);
		}
	}
}

void HazeExecuteFile::WriteInstruction(const ModuleUnit::FunctionInstruction& instruction)
{
	m_FileStream->write(HAZE_WRITE_AND_SIZE(instruction.InsCode));				//字节码

	uint32 number = (uint32)instruction.Operator.size();
	m_FileStream->write(HAZE_WRITE_AND_SIZE(number));						//操作数个数

	for (auto& iter : instruction.Operator)
	{
		s_BinaryString = WString2String(iter.Variable.Name);
		number = (uint32)s_BinaryString.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
		m_FileStream->write(s_BinaryString.data(), number);											//操作数名字

		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Scope));										//操作数作用域
		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Desc));											//操作数数据描述
		WriteType(m_FileStream, iter.Variable.Type);

		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.AddressType));									//操作数取址类型
		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Extra.Index));									//操作数索引
		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Extra.Address.Offset));							//操作数地址偏移
	}

#if HAZE_INS_LOG
	HAZE_STRING_STREAM WSS;
	WSS << "Write :" << GetInstructionString(Instruction.InsCode);
	for (auto& it : Instruction.Operator)
	{
		WSS << " ----" << it.Variable.Name << " Type :" << (uint32)it.Variable.Type.PrimaryType << " Scope: " << (uint32)it.Scope << " Base: " << it.Extra.Index
			<< " Offset: " << it.Extra.Address.Offset;
	}
	WSS << std::endl;

	std::wcout << WSS.str();
#endif
}

void HazeExecuteFile::ReadExecuteFile(HazeVM* vm)
{
	if (vm->IsDebug())
	{
		//调试用
		ReadModule(vm);
	}

	ReadGlobalDataTable(vm);
	ReadStringTable(vm);
	ReadClassTable(vm);
	ReadFunctionTable(vm);
	ReadFunctionInstruction(vm);
}

void HazeExecuteFile::ReadModule(HazeVM* vm)
{
	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	vm->m_ModuleData.resize(number);

	for (uint64 i = 0; i < vm->m_ModuleData.size(); i++)
	{
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		vm->m_ModuleData[i].Name = String2WString(s_BinaryString);

		m_InFileStream->read(HAZE_READ(vm->m_ModuleData[i].GlobalDataIndex.first));
		m_InFileStream->read(HAZE_READ(vm->m_ModuleData[i].GlobalDataIndex.second));

		m_InFileStream->read(HAZE_READ(vm->m_ModuleData[i].StringIndex.first));
		m_InFileStream->read(HAZE_READ(vm->m_ModuleData[i].StringIndex.second));

		m_InFileStream->read(HAZE_READ(vm->m_ModuleData[i].ClassIndex.first));
		m_InFileStream->read(HAZE_READ(vm->m_ModuleData[i].ClassIndex.second));

		m_InFileStream->read(HAZE_READ(vm->m_ModuleData[i].FunctionIndex.first));
		m_InFileStream->read(HAZE_READ(vm->m_ModuleData[i].FunctionIndex.second));
	}
}

void HazeExecuteFile::ReadGlobalDataTable(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::GlobalDataTable, m_States);

	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	vm->m_GlobalInitFunction.resize(number);
	for (uint64 i = 0; i < vm->m_GlobalInitFunction.size(); i++)
	{
		uint64 index = -1;
		m_InFileStream->read(HAZE_READ(index));
		vm->m_GlobalInitFunction[i] = index;
	}
	
	m_InFileStream->read(HAZE_READ(number));
	vm->m_GlobalData.resize(number);
	for (uint64 i = 0; i < vm->m_GlobalData.size(); i++)
	{
		auto& globalData = vm->m_GlobalData[i];
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		globalData.m_Name = String2WString(s_BinaryString);
		ReadType(vm, m_InFileStream, globalData.m_Type);
	}
}

void HazeExecuteFile::ReadStringTable(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::StringTable, m_States);

	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));

	auto strCount = number;
	vm->InitGlobalStringCount(number);

	for (uint64 i = 0; i < strCount; i++)
	{
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		vm->SetGlobalString(i, String2WString(s_BinaryString));
	}
}

void HazeExecuteFile::ReadClassTable(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::ClassTable, m_States);

	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	vm->m_ClassTable.resize(number);

	for (uint64 i = 0; i < vm->m_ClassTable.size(); i++)
	{
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		vm->m_ClassTable[i].Name = String2WString(s_BinaryString);

		m_InFileStream->read(HAZE_READ(vm->m_ClassTable[i].Size));

		m_InFileStream->read(HAZE_READ(number));
		vm->m_ClassTable[i].Members.resize(number);

		for (uint64 j = 0; j < vm->m_ClassTable[i].Members.size(); j++)
		{
			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			vm->m_ClassTable[i].Members[j].Variable.Name = String2WString(s_BinaryString);

			ReadType(vm, m_InFileStream, vm->m_ClassTable[i].Members[j].Variable.Type);
			m_InFileStream->read(HAZE_READ(vm->m_ClassTable[i].Members[j].Offset));
			m_InFileStream->read(HAZE_READ(vm->m_ClassTable[i].Members[j].Size));
		}

		vm->ResetSymbolClassIndex(vm->m_ClassTable[i].Name, i);
	}
}

void HazeExecuteFile::ReadFunctionTable(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::FunctionTable, m_States);

	HString name;
	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	vm->m_FunctionTable.resize(number);

	for (uint64 i = 0; i < vm->m_FunctionTable.size(); i++)
	{
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		auto iter = vm->m_ClassSymbol.find(String2WString(s_BinaryString));

		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		name = String2WString(s_BinaryString);
		vm->m_HashFunctionTable[name] = (uint32)i;
		
		if (iter != vm->m_ClassSymbol.end())
		{
			number = (uint32)(iter->first + HAZE_CLASS_FUNCTION_CONBINE).length();
			vm->m_ClassTable[iter->second].Functions[name.substr(number)] = i;
		}

		ReadType(vm, m_InFileStream, vm->m_FunctionTable[i].Type);
		m_InFileStream->read(HAZE_READ(vm->m_FunctionTable[i].FunctionDescData.Type));

		m_InFileStream->read(HAZE_READ(number));
		vm->m_FunctionTable[i].Params.resize(number);

		for (uint64 j = 0; j < vm->m_FunctionTable[i].Params.size(); j++)
		{
			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			vm->m_FunctionTable[i].Params[j].Name = String2WString(s_BinaryString);
			ReadType(vm, m_InFileStream, vm->m_FunctionTable[i].Params[j].Type);
		}

		m_InFileStream->read(HAZE_READ(number));
		vm->m_FunctionTable[i].Variables.resize(number);

		for (uint64 j = 0; j < vm->m_FunctionTable[i].Variables.size(); j++)
		{
			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			vm->m_FunctionTable[i].Variables[j].Variable.Name = String2WString(s_BinaryString);

			ReadType(vm, m_InFileStream, vm->m_FunctionTable[i].Variables[j].Variable.Type);
			m_InFileStream->read(HAZE_READ(vm->m_FunctionTable[i].Variables[j].Offset));
			m_InFileStream->read(HAZE_READ(vm->m_FunctionTable[i].Variables[j].Size));
			m_InFileStream->read(HAZE_READ(vm->m_FunctionTable[i].Variables[j].Line));
		}

		m_InFileStream->read(HAZE_READ(number));
		vm->m_FunctionTable[i].TempRegisters.resize(number);

		for (uint64 j = 0; j < vm->m_FunctionTable[i].TempRegisters.size(); j++)
		{
			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			vm->m_FunctionTable[i].TempRegisters[j].Name = String2WString(s_BinaryString);

			m_InFileStream->read(HAZE_READ(vm->m_FunctionTable[i].TempRegisters[j].Offset));
			ReadType(vm, m_InFileStream, vm->m_FunctionTable[i].TempRegisters[j].Type);
		}

		m_InFileStream->read(HAZE_READ(vm->m_FunctionTable[i].InstructionNum));
		m_InFileStream->read(HAZE_READ(vm->m_FunctionTable[i].FunctionDescData.InstructionStartAddress));

		m_InFileStream->read(HAZE_READ(vm->m_FunctionTable[i].FunctionDescData.StartLine));
		m_InFileStream->read(HAZE_READ(vm->m_FunctionTable[i].FunctionDescData.EndLine));
	}
}

void HazeExecuteFile::ReadFunctionInstruction(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::InstructionTable, m_States);

	uint64 funcInsLength = 0;
	m_InFileStream->read(HAZE_READ(funcInsLength));
	vm->m_Instructions.resize(funcInsLength);

	uint32 number = 0;
	for (uint64 i = 0; i < vm->m_Instructions.size(); i++)
	{
		m_InFileStream->read(HAZE_READ(vm->m_Instructions[i].InsCode));
		ReadInstruction(vm, vm->m_Instructions[i]);
	}

	//重新指认std lib 函数指针
	auto& stdLib = HazeStandardLibraryBase::GetStdLib();
	for (auto& iter : vm->m_HashFunctionTable)
	{
		auto& function = vm->m_FunctionTable[iter.second];
		if (function.FunctionDescData.Type == InstructionFunctionType::StaticLibFunction)
		{
			bool set = false;
			for (auto& lib : stdLib)
			{
				auto pointer = lib.second->find(iter.first);
				if (pointer != lib.second->end())
				{
					set = true;
					function.FunctionDescData.StdLibFunction = pointer->second;
				}
				
			}

			if (!set)
			{
				HAZE_LOG_ERR_W("标准库未能匹配到函数<%s>!\n", iter.first.c_str());
			}
		}
	}
}

void HazeExecuteFile::ReadInstruction(HazeVM* vm, Instruction& instruction)
{
	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	instruction.Operator.resize(number);

	for (auto& iter : instruction.Operator)
	{
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		iter.Variable.Name = String2WString(s_BinaryString);

		m_InFileStream->read(HAZE_READ(iter.Scope));
		m_InFileStream->read(HAZE_READ(iter.Desc));

		if (instruction.InsCode == InstructionOpCode::NEW)
		{
			ReadType(vm, m_InFileStream, iter.Variable.Type);

		}
		else
		{
			ReadType(vm, m_InFileStream, iter.Variable.Type);
		}
		m_InFileStream->read(HAZE_READ(iter.AddressType));

		m_InFileStream->read(HAZE_READ(iter.Extra.Index));
		m_InFileStream->read(HAZE_READ(iter.Extra.Address.Offset));	//操作数地址偏移, 指针指之属性应定义单独类型
	}

#if HAZE_INS_LOG
	HAZE_STRING_STREAM WSS;
	WSS << "Read: " << GetInstructionString(Instruction.InsCode);
	for (auto& it : Instruction.Operator)
	{
		WSS << " ----" << it.Variable.Name << " Type: " << (unsigned int)it.Variable.Type.PrimaryType << " Scope: " << (unsigned int)it.Scope << " Base: " << it.Extra.Index
			<< " Offset: " << it.Extra.Address.Offset;
	}
	WSS << std::endl;

	std::wcout << WSS.str();
#endif
}