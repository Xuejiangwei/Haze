#include <filesystem>
#include <fstream>

#include "HazeExecuteFile.h"
#include "HazeStandardLibraryBase.h"
#include "HazeVM.h"
#include "HazeFilePathHelper.h"
#include "HazeLog.h"

#define HAZE_INS_LOG			0

thread_local static HAZE_BINARY_STRING s_BinaryString;

static std::unordered_map<HazeFileFormat, const HAZE_CHAR*> s_HashMap_FileFormatString =
{
	{HazeFileFormat::GlobalDataTable, HAZE_TEXT("ȫ�����ݱ�")},
	{HazeFileFormat::StringTable, HAZE_TEXT("�ַ�����")},
	{HazeFileFormat::ClassTable, HAZE_TEXT("���")},
	{HazeFileFormat::FunctionTable, HAZE_TEXT("������")},
	{HazeFileFormat::InstructionTable, HAZE_TEXT("ָ���")},
};

static const HAZE_CHAR* GetFileFormatString(HazeFileFormat format)
{
	auto Iter = s_HashMap_FileFormatString.find(HazeFileFormat::GlobalDataTable);
	if (Iter != s_HashMap_FileFormatString.end())
	{
		return Iter->second;
	}

	HAZE_LOG_ERR(HAZE_TEXT("���ܹ��ҵ��������ļ��ĸ�ʽ<%d>!\n"), (int)format);
	return HAZE_TEXT("None");
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
					HAZE_LOG_ERR(HAZE_TEXT("����ִ���ļ�����,û������<%s>����!\n"), GetFileFormatString((HazeFileFormat)i));
				}
				else if (type == ExeFileType::In)
				{
					HAZE_LOG_ERR(HAZE_TEXT("����ִ���ļ�����,û�н���<%s>����!\n"), GetFileFormatString((HazeFileFormat)i));
				}
			}
		}

		if (State[Format])
		{
			if (type == ExeFileType::Out)
			{
				HAZE_LOG_ERR(HAZE_TEXT("����ִ���ļ�����,�ظ�����<%s>!\n"), GetFileFormatString(Format));
			}
			else if (type == ExeFileType::In)
			{
				HAZE_LOG_ERR(HAZE_TEXT("����ִ���ļ�����,�ظ�����<%s>!\n"), GetFileFormatString(Format));
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

HazeExecuteFile::HazeExecuteFile(ExeFileType type)
{
	if (type == ExeFileType::Out)
	{
		m_FileStream = std::make_unique<HAZE_BINARY_OFSTREAM>();
		//���ö����ƵĻ���д��10���ᵱ�ɻ������⴦��д�������ַ� 0x0d 0x0a�����س����з�
		m_FileStream->open(GetMainBinaryFilePath(), std::ios::out | std::ios::binary);
	}
	else if (type == ExeFileType::In)
	{
		m_InFileStream = std::make_unique<HAZE_BINARY_IFSTREAM>(GetMainBinaryFilePath(), std::ios::in | std::ios::binary);
		m_InFileStream->imbue(std::locale("chs"));
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("����Haze�������ļ�ʧ��!\n"));
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
					HAZE_LOG_ERR(HAZE_TEXT("����<%s>����!\n"), it->second);
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("���ɶ������ļ�����\n"));
				}
			}
			else
			{
				auto it = s_HashMap_FileFormatString.find((HazeFileFormat)i);
				if (it != s_HashMap_FileFormatString.end())
				{
					HAZE_LOG_ERR(HAZE_TEXT("����<%s>����!\n"), it->second);
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("�����������ļ�����\n"));
				}
			}
		}
	}
}

void HazeExecuteFile::WriteModule(const std::unordered_map<HAZE_STRING, std::shared_ptr<ModuleUnit>>& moduleUnit)
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
	auto funcInstructionLength = WriteFunctionTable(functionTable, (uint32)globalDataTable.Instructions.size());
	WriteAllInstruction(functionTable, globalDataTable, funcInstructionLength);
}

void HazeExecuteFile::WriteGlobalDataTable(const ModuleUnit::GlobalDataTable& table)
{
	FileFormatCheck check(ExeFileType::Out, HazeFileFormat::GlobalDataTable, m_States);

	uint32 number = (uint32)table.Data.size();
	m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

	number = (uint32)table.ClassObjectAllSize;
	m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

	for (auto& iter : table.Data)
	{
		number = (uint32)iter.StartAddress;
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

		number = (uint32)iter.EndAddress;
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

		s_BinaryString = WString2String(iter.Name);
		number = (uint32)s_BinaryString.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
		m_FileStream->write(s_BinaryString.c_str(), number);

		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Size));

		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Type.PrimaryType));

		if (iter.Type.NeedSecondaryType())
		{
			m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Type.SecondaryType));
		}

		if (iter.Type.NeedCustomName())
		{
			s_BinaryString = WString2String(iter.Type.CustomName);
			number = (uint32)s_BinaryString.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			m_FileStream->write(s_BinaryString.c_str(), number);
		}

		if (!(iter.Type.NeedCustomName() || iter.Type.NeedSecondaryType()))
		{
			m_FileStream->write(GetBinaryPointer(iter.Type.PrimaryType, iter.Value), iter.Size);
		}
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

			m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Members[i].Variable.Type.PrimaryType));
			m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Members[i].Variable.Type.SecondaryType));

			s_BinaryString = WString2String(HAZE_STRING(iter.Members[i].Variable.Type.CustomName));
			number = (uint32)s_BinaryString.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			m_FileStream->write(s_BinaryString.data(), number);

			m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Members[i].Offset));
			m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Members[i].Size));
		}
	}
}

uint32 HazeExecuteFile::WriteFunctionTable(const ModuleUnit::FunctionTable& table, uint32 globalDataInsLength)
{
	uint32 instructionStartAddr = globalDataInsLength;
	{
		FileFormatCheck check(ExeFileType::Out, HazeFileFormat::FunctionTable, m_States);

		uint32 number = (uint32)table.m_Functions.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));

		for (auto& function : table.m_Functions)
		{
			s_BinaryString = WString2String(function.Name);
			number = (uint32)s_BinaryString.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			m_FileStream->write(s_BinaryString.data(), number);

			m_FileStream->write(HAZE_WRITE_AND_SIZE(function.Type.PrimaryType));
			m_FileStream->write(HAZE_WRITE_AND_SIZE(function.Type.SecondaryType));
			s_BinaryString = WString2String(function.Type.CustomName);
			number = (uint32)s_BinaryString.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			m_FileStream->write(s_BinaryString.c_str(), number);

			m_FileStream->write(HAZE_WRITE_AND_SIZE(function.DescType));

			number = (uint32)function.Params.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			for (auto& iter : function.Params)
			{
				s_BinaryString = WString2String(iter.Name);
				number = (uint32)s_BinaryString.size();
				m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
				m_FileStream->write(s_BinaryString.c_str(), number);

				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Type.PrimaryType));
				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Type.SecondaryType));
				s_BinaryString = WString2String(iter.Type.CustomName);
				number = (uint32)s_BinaryString.size();
				m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
				m_FileStream->write(s_BinaryString.c_str(), number);
			}

			number = (uint32)function.Variables.size();
			m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
			for (auto& iter : function.Variables)
			{
				s_BinaryString = WString2String(iter.Variable.Name);
				number = (uint32)s_BinaryString.size();
				m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
				m_FileStream->write(s_BinaryString.c_str(), number);

				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Variable.Type.PrimaryType));
				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Variable.Type.SecondaryType));
				s_BinaryString = WString2String(iter.Variable.Type.CustomName);
				number = (uint32)s_BinaryString.size();
				m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
				m_FileStream->write(s_BinaryString.c_str(), number);

				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Offset));
				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Size));
				m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Line));
			}

			//д��ָ��������ʼ��ַ
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

void HazeExecuteFile::WriteAllInstruction(const ModuleUnit::FunctionTable& table,
	const ModuleUnit::GlobalDataTable& globalDataTable, uint32 funcInslength)
{
	FileFormatCheck check(ExeFileType::Out, HazeFileFormat::InstructionTable, m_States);

	funcInslength += (uint32)globalDataTable.Instructions.size();
	m_FileStream->write(HAZE_WRITE_AND_SIZE(funcInslength));  //ָ���ܸ���
	
	for (auto& instruction : globalDataTable.Instructions)
	{
		WriteInstruction(instruction);
	}

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
	m_FileStream->write(HAZE_WRITE_AND_SIZE(instruction.InsCode));				//�ֽ���

	uint32 number = (uint32)instruction.Operator.size();
	m_FileStream->write(HAZE_WRITE_AND_SIZE(number));						//����������

	for (auto& iter : instruction.Operator)
	{
		s_BinaryString = WString2String(iter.Variable.Name);
		number = (uint32)s_BinaryString.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
		m_FileStream->write(s_BinaryString.data(), number);											//����������

		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Scope));										//������������
		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Desc));										//��������������

		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Variable.Type.PrimaryType));					//����������
		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Variable.Type.SecondaryType));				//����������������
		s_BinaryString = WString2String(iter.Variable.Type.CustomName);
		number = (uint32)s_BinaryString.size();
		m_FileStream->write(HAZE_WRITE_AND_SIZE(number));
		m_FileStream->write(s_BinaryString.data(), number);											//������������

		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.AddressType));								//������ȡַ����

		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Extra.Index));								//����������
		m_FileStream->write(HAZE_WRITE_AND_SIZE(iter.Extra.Address.Offset));						//��������ַƫ��
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
	vm->Vector_ModuleData.resize(number);

	for (uint64 i = 0; i < vm->Vector_ModuleData.size(); i++)
	{
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		vm->Vector_ModuleData[i].Name = String2WString(s_BinaryString);

		m_InFileStream->read(HAZE_READ(vm->Vector_ModuleData[i].GlobalDataIndex.first));
		m_InFileStream->read(HAZE_READ(vm->Vector_ModuleData[i].GlobalDataIndex.second));

		m_InFileStream->read(HAZE_READ(vm->Vector_ModuleData[i].StringIndex.first));
		m_InFileStream->read(HAZE_READ(vm->Vector_ModuleData[i].StringIndex.second));

		m_InFileStream->read(HAZE_READ(vm->Vector_ModuleData[i].ClassIndex.first));
		m_InFileStream->read(HAZE_READ(vm->Vector_ModuleData[i].ClassIndex.second));

		m_InFileStream->read(HAZE_READ(vm->Vector_ModuleData[i].FunctionIndex.first));
		m_InFileStream->read(HAZE_READ(vm->Vector_ModuleData[i].FunctionIndex.second));
	}
}

void HazeExecuteFile::ReadGlobalDataTable(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::GlobalDataTable, m_States);

	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	vm->Vector_GlobalData.resize(number);
	vm->m_GlobalDataInitAddress.resize(number);

	m_InFileStream->read(HAZE_READ(number));
	vm->Vector_GlobalDataClassObjectMemory.resize(number);

	int classObjectMemoryIndex = 0;
	for (uint64 i = 0; i < vm->Vector_GlobalData.size(); i++)
	{
		vm->Vector_GlobalData[i].second = false;
		m_InFileStream->read(HAZE_READ(number));
		vm->m_GlobalDataInitAddress[i].first = number;
		m_InFileStream->read(HAZE_READ(number));
		vm->m_GlobalDataInitAddress[i].second = number;

		auto& globalData = vm->Vector_GlobalData[i].first;
		m_InFileStream->read(HAZE_READ(number));

		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		globalData.m_Name = String2WString(s_BinaryString);

		int size = 0;
		m_InFileStream->read(HAZE_READ(size));

		m_InFileStream->read((char*)&globalData.m_Type.PrimaryType, sizeof(HazeValueType));

		if (globalData.m_Type.NeedSecondaryType())
		{
			m_InFileStream->read((char*)&globalData.m_Type.SecondaryType, sizeof(HazeValueType));
		}
		if (globalData.m_Type.NeedCustomName())
		{
			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			globalData.m_Type.CustomName = String2WString(s_BinaryString);
		}

		if (!(globalData.m_Type.NeedCustomName() || globalData.m_Type.NeedSecondaryType()))
		{
			m_InFileStream->read(GetBinaryPointer(globalData.m_Type.PrimaryType, globalData.Value), size);
		}
		else
		{
			globalData.Address = &vm->Vector_GlobalDataClassObjectMemory[classObjectMemoryIndex];
			classObjectMemoryIndex += size;
		}
	}
}

void HazeExecuteFile::ReadStringTable(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::StringTable, m_States);

	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	vm->Vector_StringTable.resize(number);

	for (uint64 i = 0; i < vm->Vector_StringTable.size(); i++)
	{
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		vm->Vector_StringTable[i].second = String2WString(s_BinaryString);
	}
}

void HazeExecuteFile::ReadClassTable(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::ClassTable, m_States);

	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	vm->Vector_ClassTable.resize(number);

	for (uint64 i = 0; i < vm->Vector_ClassTable.size(); i++)
	{
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		vm->Vector_ClassTable[i].Name = String2WString(s_BinaryString);

		m_InFileStream->read(HAZE_READ(vm->Vector_ClassTable[i].Size));

		m_InFileStream->read(HAZE_READ(number));
		vm->Vector_ClassTable[i].Members.resize(number);

		for (uint64 j = 0; j < vm->Vector_ClassTable[i].Members.size(); j++)
		{
			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			vm->Vector_ClassTable[i].Members[j].Variable.Name = String2WString(s_BinaryString);

			m_InFileStream->read(HAZE_READ(vm->Vector_ClassTable[i].Members[j].Variable.Type.PrimaryType));
			m_InFileStream->read(HAZE_READ(vm->Vector_ClassTable[i].Members[j].Variable.Type.SecondaryType));
			m_InFileStream->read(HAZE_READ(number));

			m_InFileStream->read(s_BinaryString.data(), number);
			if (number > 0)
			{
				vm->Vector_ClassTable[i].Members[j].Variable.Type.CustomName = String2WString(s_BinaryString);
			}

			m_InFileStream->read(HAZE_READ(vm->Vector_ClassTable[i].Members[j].Offset));
			m_InFileStream->read(HAZE_READ(vm->Vector_ClassTable[i].Members[j].Size));
		}
	}
}

void HazeExecuteFile::ReadFunctionTable(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::FunctionTable, m_States);

	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	vm->Vector_FunctionTable.resize(number);

	for (uint64 i = 0; i < vm->Vector_FunctionTable.size(); i++)
	{
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		vm->HashMap_FunctionTable[String2WString(s_BinaryString)] = (uint32)i;

		m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].Type.PrimaryType));
		m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].Type.SecondaryType));
		m_InFileStream->read(HAZE_READ(number));
		s_BinaryString.resize(number);
		m_InFileStream->read(s_BinaryString.data(), number);
		vm->Vector_FunctionTable[i].Type.CustomName = String2WString(s_BinaryString);

		m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].FunctionDescData.Type));

		m_InFileStream->read(HAZE_READ(number));
		vm->Vector_FunctionTable[i].Params.resize(number);

		for (uint64 j = 0; j < vm->Vector_FunctionTable[i].Params.size(); j++)
		{
			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			vm->Vector_FunctionTable[i].Params[j].Name = String2WString(s_BinaryString);

			m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].Params[j].Type.PrimaryType));
			m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].Params[j].Type.SecondaryType));

			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			vm->Vector_FunctionTable[i].Params[j].Type.CustomName = String2WString(s_BinaryString);
		}

		m_InFileStream->read(HAZE_READ(number));
		vm->Vector_FunctionTable[i].Variables.resize(number);

		for (uint64 j = 0; j < vm->Vector_FunctionTable[i].Variables.size(); j++)
		{
			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			vm->Vector_FunctionTable[i].Variables[j].Variable.Name = String2WString(s_BinaryString);

			m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].Variables[j].Variable.Type.PrimaryType));
			m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].Variables[j].Variable.Type.SecondaryType));

			m_InFileStream->read(HAZE_READ(number));
			s_BinaryString.resize(number);
			m_InFileStream->read(s_BinaryString.data(), number);
			vm->Vector_FunctionTable[i].Variables[j].Variable.Type.CustomName = String2WString(s_BinaryString);

			m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].Variables[j].Offset));
			m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].Variables[j].Size));
			m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].Variables[j].Line));
		}

		m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].InstructionNum));
		m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].FunctionDescData.InstructionStartAddress));

		m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].FunctionDescData.StartLine));
		m_InFileStream->read(HAZE_READ(vm->Vector_FunctionTable[i].FunctionDescData.EndLine));
	}
}

void HazeExecuteFile::ReadFunctionInstruction(HazeVM* vm)
{
	FileFormatCheck check(ExeFileType::In, HazeFileFormat::InstructionTable, m_States);

	uint32 number = 0;
	m_InFileStream->read(HAZE_READ(number));
	vm->Instructions.resize(number);
	for (uint64 i = 0; i < vm->Instructions.size(); i++)
	{
		m_InFileStream->read(HAZE_READ(vm->Instructions[i].InsCode));
		ReadInstruction(vm->Instructions[i]);
	}

	//����ָ��std lib ����ָ��
	auto& stdLib = HazeStandardLibraryBase::GetStdLib();
	for (auto& iter : vm->HashMap_FunctionTable)
	{
		auto& function = vm->Vector_FunctionTable[iter.second];
		if (function.FunctionDescData.Type == InstructionFunctionType::StdLibFunction)
		{
			for (auto& lib : stdLib)
			{
				auto pointer = lib.second->find(iter.first);
				if (pointer != lib.second->end())
				{
					function.FunctionDescData.StdLibFunction = pointer->second;
				}
			}
		}
	}
}

void HazeExecuteFile::ReadInstruction(Instruction& instruction)
{
	uint32 unsignedInt = 0;
	m_InFileStream->read(HAZE_READ(unsignedInt));
	instruction.Operator.resize(unsignedInt);

	for (auto& iter : instruction.Operator)
	{
		m_InFileStream->read(HAZE_READ(unsignedInt));
		s_BinaryString.resize(unsignedInt);
		m_InFileStream->read(s_BinaryString.data(), unsignedInt);
		iter.Variable.Name = String2WString(s_BinaryString);

		m_InFileStream->read(HAZE_READ(iter.Scope));
		m_InFileStream->read(HAZE_READ(iter.Desc));

		m_InFileStream->read(HAZE_READ(iter.Variable.Type.PrimaryType));
		m_InFileStream->read(HAZE_READ(iter.Variable.Type.SecondaryType));
		m_InFileStream->read(HAZE_READ(unsignedInt));
		s_BinaryString.resize(unsignedInt);
		m_InFileStream->read(s_BinaryString.data(), unsignedInt);
		iter.Variable.Type.CustomName = String2WString(s_BinaryString);

		m_InFileStream->read(HAZE_READ(iter.AddressType));

		m_InFileStream->read(HAZE_READ(iter.Extra.Index));
		m_InFileStream->read(HAZE_READ(iter.Extra.Address.Offset));	//��������ַƫ��, ָ��ָ֮����Ӧ���嵥������
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