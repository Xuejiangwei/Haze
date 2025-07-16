#include "HazePch.h"
#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeFileLib.h"

static HashMap<HString, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ H_TEXT("打开文件"), &HazeFileLib::OpenFile },
	{ H_TEXT("关闭文件"), &HazeFileLib::CloseFile },

	{ H_TEXT("读取字符"), &HazeFileLib::ReadChar },
	{ H_TEXT("读取字符串"), &HazeFileLib::ReadString },
	{ H_TEXT("读取"), &HazeFileLib::Read },
	{ H_TEXT("读取一行"), &HazeFileLib::ReadLine },

	{ H_TEXT("写入字符"), &HazeFileLib::WriteChar },
	{ H_TEXT("写入字符串"), &HazeFileLib::WriteString },
	{ H_TEXT("写入"), &HazeFileLib::Write },
};

//FILE* f;
//fopen_s(&f, "C:\\Users\\WIN10\\Desktop\\ConsoleApplication1\\tt.txt", "r");
//auto c = fgetc(f);
//
//char** base, ** pointer;
//
//int* count;
//_get_stream_buffer_pointers(f, &base, &pointer, &count);

static bool Z_NoUse_HazeMemoryLib = HazeStandardLibraryBase::AddStdLib(H_TEXT("HazeFileLib"), &s_HashMap_Functions);

void HazeFileLib::InitializeLib()
{
	HazeStandardLibraryBase::AddStdLib(H_TEXT("HazeFileLib"), &s_HashMap_Functions);
}

void HazeFileLib::OpenFile(HAZE_STD_CALL_PARAM)
{
	x_HChar* filePath;
	int type;

	GET_PARAM_START();
	GET_PARAM(filePath);
	GET_PARAM(type);

	const x_HChar* mode = H_TEXT("r");
	switch (type)
	{
	case 0 :
		mode = H_TEXT("r, ccs=utf-8");
		break;
	case 1:
		mode = H_TEXT("r+, ccs=utf-8");
		break;
	case 10:
		mode = H_TEXT("w, ccs=utf-8");
		break;
	case 11:
		mode = H_TEXT("w+, ccs=utf-8");
		break;
	case 20:
		mode = H_TEXT("a, ccs=utf-8");
		break;
	case 21:
		mode = H_TEXT("a+, ccs=utf-8");
		break;
	default:
		break;
	}


	FILE* file;
	_wfopen_s(&file, filePath, mode);
	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::UInt64), file);
}

void HazeFileLib::CloseFile(HAZE_STD_CALL_PARAM)
{
	x_HChar* file;

	GET_PARAM_START();
	GET_PARAM(file);

	fclose((FILE*)file);
}

void HazeFileLib::ReadChar(HAZE_STD_CALL_PARAM)
{
	int* file;

	GET_PARAM_START();
	GET_PARAM(file);

	auto result = fgetwc((FILE*)file);

	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::Int32), result);
}

void HazeFileLib::ReadString(HAZE_STD_CALL_PARAM)
{
	int* file;
	int maxNum;
	x_HChar* str;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(maxNum);
	GET_PARAM(str);

	auto result = fgetws(str, maxNum, (FILE*)file);
	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::UInt64), result);
}

void HazeFileLib::Read(HAZE_STD_CALL_PARAM)
{
	int* file;
	x_uint64 size;
	x_uint64 count;
	void* buffer;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(size);
	GET_PARAM(count);
	GET_PARAM(buffer);

	auto result = fread(buffer, size, count, (FILE*)file);
	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::UInt64), result);
}

void HazeFileLib::ReadLine(HAZE_STD_CALL_PARAM)
{
	int* file;
	x_HChar* buffer;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(buffer);

	auto result = buffer;
	x_HChar c = fgetwc((FILE*)file);
	while (c != '\n')
	{
		*buffer = c;
		buffer++;

		c = fgetwc((FILE*)file);
	}

	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::UInt64), result);
}

void HazeFileLib::WriteChar(HAZE_STD_CALL_PARAM)
{
	x_HChar* file;
	x_HChar c;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(c);

	auto result = fputwc(c, (FILE*)file);
	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::Int32), result);
}

void HazeFileLib::WriteString(HAZE_STD_CALL_PARAM)
{
	x_HChar* file;
	x_HChar* str;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(str);

	auto result = fputws(str, (FILE*)file);
	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::Int32), result);
}

void HazeFileLib::Write(HAZE_STD_CALL_PARAM)
{
	x_HChar* file;
	x_uint64 size;
	x_uint64 count;
	void* buffer;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(size);
	GET_PARAM(count);
	GET_PARAM(buffer);

	auto result = fwrite(buffer, size, count, (FILE*)file);
	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::UInt64), result);
}