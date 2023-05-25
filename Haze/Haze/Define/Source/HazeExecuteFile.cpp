#include <filesystem>
#include <fstream>

#include "HazeExecuteFile.h"
#include "HazeFilePathHelper.h"
#include "HazeLog.h"

thread_local static HAZE_BINARY_STRING BinaryString;

HazeExecuteFile::HazeExecuteFile()
{
	//不用二进制的话，写入10，会当成换行特殊处理，写入两个字符 0x0d 0x0a，即回车换行符
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
		HAZE_LOG_ERR(HAZE_TEXT("生成执行文件错误,全局数据表已写入数据!\n"));
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
			HAZE_TO_DO(全局变量为复杂类型或自定义类类型);
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
		HAZE_LOG_ERR(HAZE_TEXT("生成执行文件错误,全局数据表已写入数据!\n"));
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
