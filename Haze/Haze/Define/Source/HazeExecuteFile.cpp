#include <filesystem>
#include <fstream>

#include "HazeExecuteFile.h"
#include "HazeFilePathHelper.h"
#include "HazeLog.h"

thread_local static HAZE_BINARY_STRING BinaryString;

HazeExecuteFile::HazeExecuteFile()
{
	//���ö����ƵĻ���д��10���ᵱ�ɻ������⴦��д�������ַ� 0x0d 0x0a�����س����з�
	FileStream.open(GetMainBinaryFilePath(), std::ios::out | std::ios::binary);

	memset(&State, 0, sizeof(State));
	
}

HazeExecuteFile::~HazeExecuteFile()
{
	if (FileStream.is_open())
	{
		FileStream.close();
	}
}

void HazeExecuteFile::WriteGlobalDataTable(const GlobalDataTable& Table)
{
	if (State[HazeFileFormat::GlobalDataTable])
	{
		HAZE_LOG_ERR(HAZE_TEXT("����ִ���ļ�����,ȫ�����ݱ���д������!\n"));
	}

	uint32 UInt = (uint32)Table.Vector_Data.size();

	FileStream.write(HAZE_WRITE_AND_SIZE(UInt));
	for (auto& Iter : Table.Vector_Data)
	{
		BinaryString = WString2String(Iter.Name);
		UInt = CAST_UINT32(BinaryString.size());
		FileStream.write(HAZE_WRITE_AND_SIZE(UInt));
		FileStream.write(BinaryString.c_str(), UInt);
		
		
		FileStream.write(HAZE_WRITE_AND_SIZE(Iter.Type.PrimaryType));

		if (Iter.Type.HasCustomName() || Iter.Type.NeedSecondaryType())
		{
			HAZE_TO_DO(ȫ�ֱ���Ϊ�������ͻ��Զ���������);
			return;
		}
		else
		{
			FileStream.write(GetBinaryPointer(Iter.Type.PrimaryType, Iter.Value), GetSizeByHazeType(Iter.Type.PrimaryType));
		}
	}

	State[HazeFileFormat::GlobalDataTable] = true;
}

void HazeExecuteFile::WriteStringTable(const StringTable& Table)
{
	if (State[HazeFileFormat::StringTable])
	{
		HAZE_LOG_ERR(HAZE_TEXT("����ִ���ļ�����,ȫ�����ݱ���д������!\n"));
	}

	uint32 UInt = (uint32)Table.Vector_String.size();
	FileStream.write(HAZE_WRITE_AND_SIZE(UInt));

	for (auto& Iter : Table.Vector_String)
	{
		BinaryString = WString2String(Iter.String);
		UInt = CAST_UINT32(BinaryString.size());
		FileStream.write(HAZE_WRITE_AND_SIZE(UInt));
		FileStream.write(BinaryString.data(), UInt);
	}


	State[HazeFileFormat::StringTable] = true;
}
