#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeFileLib.h"

static std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ HAZE_TEXT("打开文件"), &HazeFileLib::OpenFile },
	{ HAZE_TEXT("关闭文件"), &HazeFileLib::CloseFile },

	{ HAZE_TEXT("读取字符"), &HazeFileLib::ReadChar },
	{ HAZE_TEXT("读取字符串"), &HazeFileLib::ReadString },
	{ HAZE_TEXT("读取"), &HazeFileLib::Read },

	{ HAZE_TEXT("写入字符"), &HazeFileLib::WriteChar },
	{ HAZE_TEXT("写入字符串"), &HazeFileLib::WriteString },
	{ HAZE_TEXT("写入"), &HazeFileLib::Write },
};

//FILE* f;
//fopen_s(&f, "C:\\Users\\WIN10\\Desktop\\ConsoleApplication1\\tt.txt", "r");
//auto c = fgetc(f);
//
//char** base, ** pointer;
//
//int* count;
//_get_stream_buffer_pointers(f, &base, &pointer, &count);

static bool Z_NoUse_HazeMemoryLib = HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeFileLib"), &s_HashMap_Functions);

void HazeFileLib::InitializeLib()
{
	HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeFileLib"), &s_HashMap_Functions);
}

void HazeFileLib::OpenFile(HAZE_STD_CALL_PARAM)
{
	HAZE_CHAR* filePath;
	int type;

	GET_PARAM_START();
	GET_PARAM(filePath);
	GET_PARAM(type);

	const HAZE_CHAR* mode = HAZE_TEXT("r");
	switch (type)
	{
	case 0 :
		mode = HAZE_TEXT("r, ccs=utf-8");
		break;
	case 1:
		mode = HAZE_TEXT("r+, ccs=utf-8");
		break;
	case 10:
		mode = HAZE_TEXT("w, ccs=utf-8");
		break;
	case 11:
		mode = HAZE_TEXT("w+, ccs=utf-8");
		break;
	case 20:
		mode = HAZE_TEXT("a, ccs=utf-8");
		break;
	case 21:
		mode = HAZE_TEXT("a+, ccs=utf-8");
		break;
	default:
		break;
	}


	FILE* file;
	_wfopen_s(&file, filePath, mode);
	SET_RET_BY_TYPE(HazeValueType::PointerBase, file);
}

void HazeFileLib::CloseFile(HAZE_STD_CALL_PARAM)
{
	HAZE_CHAR* file;

	GET_PARAM_START();
	GET_PARAM(file);

	fclose((FILE*)file);
}

void HazeFileLib::ReadChar(HAZE_STD_CALL_PARAM)
{
	HAZE_CHAR* file;

	GET_PARAM_START();
	GET_PARAM(file);

	auto result = fgetwc((FILE*)file);

	SET_RET_BY_TYPE(HazeValueType::Int, result);
}

void HazeFileLib::ReadString(HAZE_STD_CALL_PARAM)
{
	HAZE_CHAR* file;
	int maxNum;
	HAZE_CHAR* str;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(maxNum);
	GET_PARAM(str);

	auto result = fgetws(str, maxNum, (FILE*)file);
	SET_RET_BY_TYPE(HazeValueType::PointerBase, result);
}

void HazeFileLib::Read(HAZE_STD_CALL_PARAM)
{
	HAZE_CHAR* file;
	uint64 size;
	uint64 count;
	void* buffer;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(size);
	GET_PARAM(count);
	GET_PARAM(buffer);

	auto result = fread(buffer, size, count, (FILE*)file);
	SET_RET_BY_TYPE(HazeValueType::UnsignedLong, result);
}

void HazeFileLib::WriteChar(HAZE_STD_CALL_PARAM)
{
	HAZE_CHAR* file;
	HAZE_CHAR c;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(c);

	auto result = fputwc(c, (FILE*)file);
	SET_RET_BY_TYPE(HazeValueType::Int, result);
}

void HazeFileLib::WriteString(HAZE_STD_CALL_PARAM)
{
	HAZE_CHAR* file;
	HAZE_CHAR* str;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(str);

	auto result = fputws(str, (FILE*)file);
	SET_RET_BY_TYPE(HazeValueType::Int, result);
}

void HazeFileLib::Write(HAZE_STD_CALL_PARAM)
{
	HAZE_CHAR* file;
	uint64 size;
	uint64 count;
	void* buffer;

	GET_PARAM_START();
	GET_PARAM(file);
	GET_PARAM(size);
	GET_PARAM(count);
	GET_PARAM(buffer);

	auto result = fwrite(buffer, size, count, (FILE*)file);
	SET_RET_BY_TYPE(HazeValueType::UnsignedLong, result);
}