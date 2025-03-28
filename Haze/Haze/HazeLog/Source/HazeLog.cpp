#include "HazePch.h"
#include <stdio.h>
#include <mutex>
#include <windows.h>
#include "HazeLog.h"

static std::mutex g_LogMutex;

static void SetSystemColor(int type = 0)
{
	switch (type)
	{
	case HazeLog::Info:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),/* FOREGROUND_INTENSITY | FOREGROUND_RED |*/ FOREGROUND_GREEN /*| FOREGROUND_BLUE*/);
		break;
	case HazeLog::Warning:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		break;
	case HazeLog::Error:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		break;
	default:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	}
}

void HazeLog::LogInfo(int type, const x_HChar* format, ...)
{
	std::lock_guard lock(g_LogMutex);

	SetSystemColor(type);
	va_list st;
	va_start(st, format);
	vwprintf(format, st);
	//vfprintf(stdout, (char*)format, st);
	va_end(st);
	SetSystemColor();
}

void HazeLog::LogInfo(int type, const char* format, ...)
{
	std::lock_guard lock(g_LogMutex);

	SetSystemColor(type);
	va_list st;
	va_start(st, format);
	vfprintf(stdout, format, st);
	va_end(st);
	SetSystemColor();
}

//void HazeLog::LogInfo(int type, const HChar* format)
//{
//	SetSystemColor(type);
//	std::cout << format;
//	SetSystemColor();
//}