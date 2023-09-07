#include <filesystem>
#include <fstream>

#include "HazeExecuteFile.h"

#include "HazeVM.h"
#include "HazeFilePathHelper.h"
#include "HazeLog.h"

#define HAZE_INS_LOG			0

extern std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> Hash_MapStdLib;

thread_local static HAZE_BINARY_STRING BinaryString;

static std::unordered_map<HazeFileFormat, const HAZE_CHAR*> HashMap_FileFormatString =
{
	{HazeFileFormat::GlobalDataTable, HAZE_TEXT("全局数据表")},
	{HazeFileFormat::StringTable, HAZE_TEXT("字符串表")},
	{HazeFileFormat::ClassTable, HAZE_TEXT("类表")},
	{HazeFileFormat::FunctionTable, HAZE_TEXT("函数表")},
	{HazeFileFormat::InstructionTable, HAZE_TEXT("指令表")},
};

static const HAZE_CHAR* GetFileFormatString(HazeFileFormat Format)
{
	auto Iter = HashMap_FileFormatString.find(HazeFileFormat::GlobalDataTable);
	if (Iter != HashMap_FileFormatString.end())
	{
		return Iter->second;
	}

	HAZE_LOG_ERR(HAZE_TEXT("不能够找到二进制文件的格式<%d>!\n"), (int)Format);
	return HAZE_TEXT("None");
}

struct FileFormatCheck
{
	FileFormatCheck(ExeFileType Type, HazeFileFormat F, bool* S) : Format(F), State(S)
	{
		for (size_t i = 0; i < Format; i++)
		{
			if (!State[i] && i < Format)
			{
				if (Type == ExeFileType::Out)
				{
					HAZE_LOG_ERR(HAZE_TEXT("生成执行文件错误,没有生成<%s>数据!\n"), GetFileFormatString((HazeFileFormat)i));
				}
				else if (Type == ExeFileType::In)
				{
					HAZE_LOG_ERR(HAZE_TEXT("解析执行文件错误,没有解析<%s>数据!\n"), GetFileFormatString((HazeFileFormat)i));
				}
			}
		}

		if (State[Format])
		{
			if (Type == ExeFileType::Out)
			{
				HAZE_LOG_ERR(HAZE_TEXT("生成执行文件错误,重复生成<%s>!\n"), GetFileFormatString(Format));
			}
			else if (Type == ExeFileType::In)
			{
				HAZE_LOG_ERR(HAZE_TEXT("解析执行文件错误,重复解析<%s>!\n"), GetFileFormatString(Format));
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

HazeExecuteFile::HazeExecuteFile(ExeFileType Type)
{
	if (Type == ExeFileType::Out)
	{
		FileStream = std::make_unique<HAZE_BINARY_OFSTREAM>();
		//不用二进制的话，写入10，会当成换行特殊处理，写入两个字符 0x0d 0x0a，即回车换行符
		FileStream->open(GetMainBinaryFilePath(), std::ios::out | std::ios::binary);
	}
	else if (Type == ExeFileType::In)
	{
		InFileStream = std::make_unique<HAZE_BINARY_IFSTREAM>(GetMainBinaryFilePath(), std::ios::in | std::ios::binary);
		InFileStream->imbue(std::locale("chs"));
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("处理Haze二进制文件失败!\n"));
	}

	memset(&State, 0, sizeof(State));
}

HazeExecuteFile::~HazeExecuteFile()
{
	if (FileStream && FileStream->is_open())
	{
		FileStream->close();
	}

	if (InFileStream && InFileStream->is_open())
	{
		InFileStream->close();
	}

	CheckAll();
}

void HazeExecuteFile::CheckAll()
{
	for (size_t i = 0; i < HazeFileFormat::End; i++)
	{
		if (!State[i])
		{
			if (FileStream)
			{
				auto It = HashMap_FileFormatString.find((HazeFileFormat)i);
				if (It != HashMap_FileFormatString.end())
				{
					HAZE_LOG_ERR(HAZE_TEXT("生成<%s>错误!\n"), It->second);
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("生成二进制文件错误\n"));
				}
			}
			else
			{
				auto It = HashMap_FileFormatString.find((HazeFileFormat)i);
				if (It != HashMap_FileFormatString.end())
				{
					HAZE_LOG_ERR(HAZE_TEXT("解析<%s>错误!\n"), It->second);
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("解析二进制文件错误\n"));
				}
			}
		}
	}
}

void HazeExecuteFile::WriteModule(const std::unordered_map<HAZE_STRING, std::shared_ptr<ModuleUnit>>& Module)
{
	uint32 GlobalDataIndex = 0;
	uint32 StringIndex = 0;
	uint32 ClassIndex = 0;
	uint32 FunctionIndex = 0;

	uint32 UInt = (uint32)Module.size();
	FileStream->write(HAZE_WRITE_AND_SIZE(UInt));

	for (auto& Iter : Module)
	{
		BinaryString = WString2String(Iter.first);
		UInt = (uint32)BinaryString.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
		FileStream->write(BinaryString.c_str(), UInt);

		FileStream->write(HAZE_WRITE_AND_SIZE(GlobalDataIndex));
		GlobalDataIndex += (uint32)Iter.second->Table_GlobalData.Vector_Data.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(GlobalDataIndex));

		FileStream->write(HAZE_WRITE_AND_SIZE(StringIndex));
		StringIndex += (uint32)Iter.second->Table_String.Vector_String.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(StringIndex));

		FileStream->write(HAZE_WRITE_AND_SIZE(ClassIndex));
		ClassIndex += (uint32)Iter.second->Table_Class.Vector_Class.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(ClassIndex));

		FileStream->write(HAZE_WRITE_AND_SIZE(FunctionIndex));
		FunctionIndex += (uint32)Iter.second->Table_Function.Vector_Function.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(FunctionIndex));
	}
}

void HazeExecuteFile::WriteExecuteFile(const ModuleUnit::GlobalDataTable& GlobalDataTable, const ModuleUnit::StringTable& StringTable,
	const ModuleUnit::ClassTable& ClassTable, const ModuleUnit::FunctionTable& FunctionTable)
{
	WriteGlobalDataTable(GlobalDataTable);
	WriteStringTable(StringTable);
	WriteClassTable(ClassTable);
	WriteFunctionTable(FunctionTable);
}

void HazeExecuteFile::WriteGlobalDataTable(const ModuleUnit::GlobalDataTable& Table)
{
	FileFormatCheck Check(ExeFileType::Out, HazeFileFormat::GlobalDataTable, State);

	uint32 UInt = (uint32)Table.Vector_Data.size();
	FileStream->write(HAZE_WRITE_AND_SIZE(UInt));

	UInt = (uint32)Table.ClassObjectAllSize;
	FileStream->write(HAZE_WRITE_AND_SIZE(UInt));

	for (auto& Iter : Table.Vector_Data)
	{
		BinaryString = WString2String(Iter.Name);
		UInt = (uint32)BinaryString.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
		FileStream->write(BinaryString.c_str(), UInt);

		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Size));

		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Type.PrimaryType));

		if (Iter.Type.NeedSecondaryType())
		{
			FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Type.SecondaryType));
		}

		if (Iter.Type.NeedCustomName())
		{
			BinaryString = WString2String(Iter.Type.CustomName);
			UInt = (uint32)BinaryString.size();
			FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
			FileStream->write(BinaryString.c_str(), UInt);
		}

		if (!(Iter.Type.NeedCustomName() || Iter.Type.NeedSecondaryType()))
		{
			FileStream->write(GetBinaryPointer(Iter.Type.PrimaryType, Iter.Value), Iter.Size);
		}
	}
}

void HazeExecuteFile::WriteStringTable(const ModuleUnit::StringTable& Table)
{
	FileFormatCheck Check(ExeFileType::Out, HazeFileFormat::StringTable, State);

	uint32 UInt = (uint32)Table.Vector_String.size();
	FileStream->write(HAZE_WRITE_AND_SIZE(UInt));

	for (auto& Iter : Table.Vector_String)
	{
		BinaryString = WString2String(Iter.String);
		UInt = (uint32)BinaryString.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
		FileStream->write(BinaryString.data(), UInt);
	}
}

void HazeExecuteFile::WriteClassTable(const ModuleUnit::ClassTable& Table)
{
	FileFormatCheck Check(ExeFileType::Out, HazeFileFormat::ClassTable, State);

	uint32 UInt = (uint32)Table.Vector_Class.size();
	FileStream->write(HAZE_WRITE_AND_SIZE(UInt));

	for (auto& Iter : Table.Vector_Class)
	{
		BinaryString = WString2String(Iter.Name);
		UInt = (uint32)BinaryString.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
		FileStream->write(BinaryString.data(), UInt);

		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Size));

		UInt = (uint32)Iter.Vector_Member.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(UInt));

		for (size_t i = 0; i < Iter.Vector_Member.size(); i++)
		{
			BinaryString = WString2String(Iter.Vector_Member[i].Variable.Name);
			UInt = (uint32)BinaryString.size();
			FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
			FileStream->write(BinaryString.data(), UInt);

			FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Vector_Member[i].Variable.Type.PrimaryType));
			FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Vector_Member[i].Variable.Type.SecondaryType));

			BinaryString = WString2String(HAZE_STRING(Iter.Vector_Member[i].Variable.Type.CustomName));
			UInt = (uint32)BinaryString.size();
			FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
			FileStream->write(BinaryString.data(), UInt);

			FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Vector_Member[i].Offset));
			FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Vector_Member[i].Size));
		}
	}
}

void HazeExecuteFile::WriteFunctionTable(const ModuleUnit::FunctionTable& Table)
{
	uint32 InstructionStartAddr = 0;
	{
		FileFormatCheck Check(ExeFileType::Out, HazeFileFormat::FunctionTable, State);

		uint32 UInt = (uint32)Table.Vector_Function.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(UInt));

		for (auto& Function : Table.Vector_Function)
		{
			BinaryString = WString2String(Function.Name);
			UInt = (uint32)BinaryString.size();
			FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
			FileStream->write(BinaryString.data(), UInt);

			FileStream->write(HAZE_WRITE_AND_SIZE(Function.Type));
			FileStream->write(HAZE_WRITE_AND_SIZE(Function.DescType));

			UInt = (uint32)Function.Vector_Param.size();
			FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
			for (auto& Iter : Function.Vector_Param)
			{
				BinaryString = WString2String(Iter.Name);
				UInt = (uint32)BinaryString.size();
				FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
				FileStream->write(BinaryString.c_str(), UInt);

				FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Type.PrimaryType));
				FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Type.SecondaryType));
				BinaryString = WString2String(Iter.Type.CustomName);
				UInt = (uint32)BinaryString.size();
				FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
				FileStream->write(BinaryString.c_str(), UInt);
			}

			UInt = (uint32)Function.Vector_Variable.size();
			FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
			for (auto& Iter : Function.Vector_Variable)
			{
				BinaryString = WString2String(Iter.Variable.Name);
				UInt = (uint32)BinaryString.size();
				FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
				FileStream->write(BinaryString.c_str(), UInt);

				FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Variable.Type.PrimaryType));
				FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Variable.Type.SecondaryType));
				BinaryString = WString2String(Iter.Variable.Type.CustomName);
				UInt = (uint32)BinaryString.size();
				FileStream->write(HAZE_WRITE_AND_SIZE(UInt));
				FileStream->write(BinaryString.c_str(), UInt);

				FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Offset));
				FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Size));
				FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Line));
			}

			//写入指令数与起始地址
			UInt = (uint32)Function.Vector_Instruction.size();
			FileStream->write(HAZE_WRITE_AND_SIZE(UInt));

			FileStream->write(HAZE_WRITE_AND_SIZE(InstructionStartAddr));
			InstructionStartAddr += UInt;
		}
	}

	WriteFunctionInstruction(Table, InstructionStartAddr);
}

void HazeExecuteFile::WriteFunctionInstruction(const ModuleUnit::FunctionTable& Table, uint32 Length)
{
	FileFormatCheck Check(ExeFileType::Out, HazeFileFormat::InstructionTable, State);

	FileStream->write(HAZE_WRITE_AND_SIZE(Length));  //指令总个数
	for (auto& Function : Table.Vector_Function)
	{
		for (auto& Ins : Function.Vector_Instruction)
		{
			WriteInstruction(Ins);
		}
	}
}

void HazeExecuteFile::WriteInstruction(const ModuleUnit::FunctionInstruction& Instruction)
{
	FileStream->write(HAZE_WRITE_AND_SIZE(Instruction.InsCode));				//字节码

	uint32 Uint = (uint32)Instruction.Operator.size();
	FileStream->write(HAZE_WRITE_AND_SIZE(Uint));						//操作数个数

	for (auto& Iter : Instruction.Operator)
	{
		BinaryString = WString2String(Iter.Variable.Name);
		Uint = (uint32)BinaryString.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(Uint));
		FileStream->write(BinaryString.data(), Uint);											//操作数名字

		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Scope));										//操作数作用域
		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Desc));										//操作数数据描述

		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Variable.Type.PrimaryType));					//操作数类型
		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Variable.Type.SecondaryType));				//操作数类型子类型
		BinaryString = WString2String(Iter.Variable.Type.CustomName);
		Uint = (uint32)BinaryString.size();
		FileStream->write(HAZE_WRITE_AND_SIZE(Uint));
		FileStream->write(BinaryString.data(), Uint);											//操作数类型名

		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.AddressType));								//操作数取址类型

		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Extra.Index));								//操作数索引
		FileStream->write(HAZE_WRITE_AND_SIZE(Iter.Extra.Address.Offset));						//操作数地址偏移
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

void HazeExecuteFile::ReadExecuteFile(HazeVM* VM)
{
	if (VM->IsDebug())
	{
		ReadModule(VM);
	}

	ReadGlobalDataTable(VM);
	ReadStringTable(VM);
	ReadClassTable(VM);
	ReadFunctionTable(VM);
	ReadFunctionInstruction(VM);
}

void HazeExecuteFile::ReadModule(HazeVM* VM)
{
	uint32 Num = 0;
	InFileStream->read(HAZE_READ(Num));
	VM->Vector_ModuleData.resize(Num);

	for (uint64 i = 0; i < VM->Vector_ModuleData.size(); i++)
	{
		InFileStream->read(HAZE_READ(Num));
		BinaryString.resize(Num);
		InFileStream->read(BinaryString.data(), Num);
		VM->Vector_ModuleData[i].Name = String2WString(BinaryString);

		InFileStream->read(HAZE_READ(VM->Vector_ModuleData[i].GlobalDataIndex.first));
		InFileStream->read(HAZE_READ(VM->Vector_ModuleData[i].GlobalDataIndex.second));

		InFileStream->read(HAZE_READ(VM->Vector_ModuleData[i].StringIndex.first));
		InFileStream->read(HAZE_READ(VM->Vector_ModuleData[i].StringIndex.second));

		InFileStream->read(HAZE_READ(VM->Vector_ModuleData[i].ClassIndex.first));
		InFileStream->read(HAZE_READ(VM->Vector_ModuleData[i].ClassIndex.second));

		InFileStream->read(HAZE_READ(VM->Vector_ModuleData[i].FunctionIndex.first));
		InFileStream->read(HAZE_READ(VM->Vector_ModuleData[i].FunctionIndex.second));
	}
}

void HazeExecuteFile::ReadGlobalDataTable(HazeVM* VM)
{
	FileFormatCheck Check(ExeFileType::In, HazeFileFormat::GlobalDataTable, State);

	uint32 Num = 0;
	InFileStream->read(HAZE_READ(Num));
	VM->Vector_GlobalData.resize(Num);

	InFileStream->read(HAZE_READ(Num));
	VM->Vector_GlobalDataClassObjectMemory.resize(Num);

	int ClassObjectMemoryIndex = 0;
	for (uint64 i = 0; i < VM->Vector_GlobalData.size(); i++)
	{
		InFileStream->read(HAZE_READ(Num));

		BinaryString.resize(Num);
		InFileStream->read(BinaryString.data(), Num);
		VM->Vector_GlobalData[i].Name = String2WString(BinaryString);

		int Size = 0;
		InFileStream->read(HAZE_READ(Size));

		InFileStream->read((char*)&VM->Vector_GlobalData[i].Type.PrimaryType, sizeof(HazeValueType));

		if (VM->Vector_GlobalData[i].Type.NeedSecondaryType())
		{
			InFileStream->read((char*)&VM->Vector_GlobalData[i].Type.SecondaryType, sizeof(HazeValueType));
		}
		if (VM->Vector_GlobalData[i].Type.NeedCustomName())
		{
			InFileStream->read(HAZE_READ(Num));
			BinaryString.resize(Num);
			InFileStream->read(BinaryString.data(), Num);
			VM->Vector_GlobalData[i].Type.CustomName = String2WString(BinaryString);
		}

		if (!(VM->Vector_GlobalData[i].Type.NeedCustomName() || VM->Vector_GlobalData[i].Type.NeedSecondaryType()))
		{
			InFileStream->read(GetBinaryPointer(VM->Vector_GlobalData[i].Type.PrimaryType, VM->Vector_GlobalData[i].Value), Size);
		}
		else
		{
			VM->Vector_GlobalData[i].Address = &VM->Vector_GlobalDataClassObjectMemory[ClassObjectMemoryIndex];
			ClassObjectMemoryIndex += Size;
		}
	}
}

void HazeExecuteFile::ReadStringTable(HazeVM* VM)
{
	FileFormatCheck Check(ExeFileType::In, HazeFileFormat::StringTable, State);

	uint32 Num = 0;
	InFileStream->read(HAZE_READ(Num));
	VM->Vector_StringTable.resize(Num);

	for (uint64 i = 0; i < VM->Vector_StringTable.size(); i++)
	{
		InFileStream->read(HAZE_READ(Num));
		BinaryString.resize(Num);
		InFileStream->read(BinaryString.data(), Num);
		VM->Vector_StringTable[i].second = String2WString(BinaryString);
	}
}

void HazeExecuteFile::ReadClassTable(HazeVM* VM)
{
	FileFormatCheck Check(ExeFileType::In, HazeFileFormat::ClassTable, State);

	uint32 Num = 0;
	InFileStream->read(HAZE_READ(Num));
	VM->Vector_ClassTable.resize(Num);

	for (uint64 i = 0; i < VM->Vector_ClassTable.size(); i++)
	{
		InFileStream->read(HAZE_READ(Num));
		BinaryString.resize(Num);
		InFileStream->read(BinaryString.data(), Num);
		VM->Vector_ClassTable[i].Name = String2WString(BinaryString);

		InFileStream->read(HAZE_READ(VM->Vector_ClassTable[i].Size));

		InFileStream->read(HAZE_READ(Num));
		VM->Vector_ClassTable[i].Vector_Member.resize(Num);

		for (uint64 j = 0; j < VM->Vector_ClassTable[i].Vector_Member.size(); j++)
		{
			InFileStream->read(HAZE_READ(Num));
			BinaryString.resize(Num);
			InFileStream->read(BinaryString.data(), Num);
			VM->Vector_ClassTable[i].Vector_Member[j].Variable.Name = String2WString(BinaryString);

			InFileStream->read(HAZE_READ(VM->Vector_ClassTable[i].Vector_Member[j].Variable.Type.PrimaryType));
			InFileStream->read(HAZE_READ(VM->Vector_ClassTable[i].Vector_Member[j].Variable.Type.SecondaryType));
			InFileStream->read(HAZE_READ(Num));

			InFileStream->read(BinaryString.data(), Num);
			if (Num > 0)
			{
				VM->Vector_ClassTable[i].Vector_Member[j].Variable.Type.CustomName = String2WString(BinaryString);
			}

			InFileStream->read(HAZE_READ(VM->Vector_ClassTable[i].Vector_Member[j].Offset));
			InFileStream->read(HAZE_READ(VM->Vector_ClassTable[i].Vector_Member[j].Size));
		}
	}
}

void HazeExecuteFile::ReadFunctionTable(HazeVM* VM)
{
	FileFormatCheck Check(ExeFileType::In, HazeFileFormat::FunctionTable, State);

	uint32 Num = 0;
	InFileStream->read(HAZE_READ(Num));
	VM->Vector_FunctionTable.resize(Num);

	for (uint64 i = 0; i < VM->Vector_FunctionTable.size(); i++)
	{
		InFileStream->read(HAZE_READ(Num));
		BinaryString.resize(Num);
		InFileStream->read(BinaryString.data(), Num);
		VM->HashMap_FunctionTable[String2WString(BinaryString)] = (uint32)i;

		InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].Type));
		InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].FunctionDescData.Type));

		InFileStream->read(HAZE_READ(Num));
		VM->Vector_FunctionTable[i].Vector_Param.resize(Num);

		for (uint64 j = 0; j < VM->Vector_FunctionTable[i].Vector_Param.size(); j++)
		{
			InFileStream->read(HAZE_READ(Num));
			BinaryString.resize(Num);
			InFileStream->read(BinaryString.data(), Num);
			VM->Vector_FunctionTable[i].Vector_Param[j].Name = String2WString(BinaryString);

			InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].Vector_Param[j].Type.PrimaryType));
			InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].Vector_Param[j].Type.SecondaryType));

			InFileStream->read(HAZE_READ(Num));
			BinaryString.resize(Num);
			InFileStream->read(BinaryString.data(), Num);
			VM->Vector_FunctionTable[i].Vector_Param[j].Type.CustomName = String2WString(BinaryString);
		}

		InFileStream->read(HAZE_READ(Num));
		VM->Vector_FunctionTable[i].Vector_Variable.resize(Num);

		for (uint64 j = 0; j < VM->Vector_FunctionTable[i].Vector_Variable.size(); j++)
		{
			InFileStream->read(HAZE_READ(Num));
			BinaryString.resize(Num);
			InFileStream->read(BinaryString.data(), Num);
			VM->Vector_FunctionTable[i].Vector_Variable[j].Variable.Name = String2WString(BinaryString);

			InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].Vector_Variable[j].Variable.Type.PrimaryType));
			InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].Vector_Variable[j].Variable.Type.SecondaryType));

			InFileStream->read(HAZE_READ(Num));
			BinaryString.resize(Num);
			InFileStream->read(BinaryString.data(), Num);
			VM->Vector_FunctionTable[i].Vector_Variable[j].Variable.Type.CustomName = String2WString(BinaryString);

			InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].Vector_Variable[j].Offset));
			InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].Vector_Variable[j].Size));
			InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].Vector_Variable[j].Line));
		}

		InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].InstructionNum));
		InFileStream->read(HAZE_READ(VM->Vector_FunctionTable[i].FunctionDescData.InstructionStartAddress));
	}
}

void HazeExecuteFile::ReadFunctionInstruction(HazeVM* VM)
{
	FileFormatCheck Check(ExeFileType::In, HazeFileFormat::InstructionTable, State);

	uint32 Num = 0;
	InFileStream->read(HAZE_READ(Num));
	VM->Vector_Instruction.resize(Num);
	for (uint64 i = 0; i < VM->Vector_Instruction.size(); i++)
	{
		InFileStream->read(HAZE_READ(VM->Vector_Instruction[i].InsCode));
		ReadInstruction(VM->Vector_Instruction[i]);
	}

	//重新指认std lib 函数指针
	for (auto& Iter : VM->HashMap_FunctionTable)
	{
		auto& Function = VM->Vector_FunctionTable[Iter.second];
		if (Function.FunctionDescData.Type == InstructionFunctionType::StdLibFunction)
		{
			for (auto& I : Hash_MapStdLib)
			{
				auto P = I.second->find(Iter.first);
				if (P != I.second->end())
				{
					Function.FunctionDescData.StdLibFunction = P->second;
				}
			}
		}
	}
}

void HazeExecuteFile::ReadInstruction(Instruction& Instruction)
{
	uint32 UnsignedInt = 0;
	InFileStream->read(HAZE_READ(UnsignedInt));
	Instruction.Operator.resize(UnsignedInt);

	for (auto& Iter : Instruction.Operator)
	{
		InFileStream->read(HAZE_READ(UnsignedInt));
		BinaryString.resize(UnsignedInt);
		InFileStream->read(BinaryString.data(), UnsignedInt);
		Iter.Variable.Name = String2WString(BinaryString);

		InFileStream->read(HAZE_READ(Iter.Scope));
		InFileStream->read(HAZE_READ(Iter.Desc));

		InFileStream->read(HAZE_READ(Iter.Variable.Type.PrimaryType));
		InFileStream->read(HAZE_READ(Iter.Variable.Type.SecondaryType));
		InFileStream->read(HAZE_READ(UnsignedInt));
		BinaryString.resize(UnsignedInt);
		InFileStream->read(BinaryString.data(), UnsignedInt);
		Iter.Variable.Type.CustomName = String2WString(BinaryString);

		InFileStream->read(HAZE_READ(Iter.AddressType));

		InFileStream->read(HAZE_READ(Iter.Extra.Index));
		InFileStream->read(HAZE_READ(Iter.Extra.Address.Offset));	//操作数地址偏移, 指针指之属性应定义单独类型
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